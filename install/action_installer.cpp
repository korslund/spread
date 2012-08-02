#include "action_installer.hpp"
#include "rules/urlrule.hpp"
#include "rules/arcrule.hpp"
#include "job/thread.hpp"
#include "htasks/unpackhash.hpp"
#include "htasks/downloadhash.hpp"
#include "htasks/copyhash.hpp"
#include <stdexcept>

using namespace Spread;
using namespace Jobs;

enum Type
  {
    TT_FileCopy,
    TT_Download,
    TT_Unpack
  };

enum Status
  {
    TS_Unwanted,      // Nobody has requested this target
    TS_Wanted,        // This target is requested
    TS_Waiting,       // We are waiting for a dependency to finish
    TS_Ready,         // No unfinished dependencies, ready to run job
    TS_Running,       // Job is currently executing
    TS_Done           // Job has finished successfully
  };

struct Target;
typedef std::map<Hash,Target> TargetMap;

struct Target
{
  Hash hash;
  const Action *action;
  std::string url; // For download actions only
  DirectoryCPtr dir; // For unpack actions only

  int type, status;

  ActionInstaller *owner;

  // Dependencies we are waiting for, and their final locations when
  // done.
  typedef std::map<Hash, Target*> HTMap;
  HTMap waitingFor;

  JobInfoPtr info;

  // List of all output files. If there are no directly requested
  // outputs, but the target is implicitly required by other
  // targets, this list will contain a tmp file instead.
  std::vector<std::string> outFiles;

  // Targets that depend on us should use this file as their input
  // when we are TS_Done.
  std::string useForDeps;

  ~Target()
  {
    if(info && info->isBusy()) info->abort();
  }

  // Check where we are at, and move status forward if possible.
  void update(int64_t &cur, int64_t &tot)
  {
    assert(status == TS_Waiting || status == TS_Ready ||
           status == TS_Running || status == TS_Done);

    if(info)
      {
        cur += info->getCurrent();
        tot += info->getTotal();
      }

    if(status == TS_Waiting)
      checkDeps();

    if(status == TS_Ready)
      {
        startJob();
        assert(status == TS_Running || status == TS_Done);
      }

    if(status == TS_Running)
      checkJob();

  }

  // Check all our dependencies are done. If so, move our status from
  // TS_Waiting to TS_Ready.
  void checkDeps()
  {
    assert(status == TS_Waiting);

    HTMap::iterator it;
    for(it = waitingFor.begin(); it != waitingFor.end(); it++)
      {
        const Target *t = it->second;
        assert(t);
        if(t->status != TS_Done)
          return;
      }

    // If we came this far, all dependencies are TS_Done. That means
    // we are aready.
    status = TS_Ready;
  }

  void startJob()
  {
    assert(status == TS_Ready);
    assert(!info);

    HashTask *htask;
    if(type == TT_FileCopy)
      htask = new CopyHash(action->source);
    else if(type == TT_Download)
      htask = new DownloadHash(url);
    else if(type == TT_Unpack)
      htask = new UnpackHash(dir->dir);
    else assert(0);

    assert(htask);

    // Add all our dependencies as task inputs
    {
      HTMap::iterator it;
      for(it = waitingFor.begin(); it != waitingFor.end(); it++)
        {
          const Hash &h = it->first;
          const Target &t = *it->second;

          // Use the file that the Target has chosen for us
          assert(t.useForDeps != "");
          htask->addInput(h, t.useForDeps);
        }
    }

    // If we have no outputs, then we are done. This is only allowed
    // for empty copy operations (which are basically just markers
    // telling us about already existing files.)
    if(outFiles.size() == 0)
      {
        assert(type == TT_FileCopy);
        status = TS_Done;
        return;
      }

    // Add all our requests as task outputs
    assert(outFiles.size());
    for(int i=0; i<outFiles.size(); i++)
      htask->addOutput(hash, outFiles[i]);

    info = htask->getInfo();
    assert(info);
    Thread::run(htask);
    status = TS_Running;
  }

  void checkJob()
  {
    assert(status == TS_Running);

    assert(info);
    if(info->isFinished())
      {
        // Check for recoverable errors
        if(info->isError() && type == TT_Download)
          {
            // Try to get a replacement URL
            assert(url != "");
            url = owner->brokenURL(hash, url);
            if(url != "")
              {
                // There was a replacement! Silently take one step
                // back to TS_Ready, pretend the botched job never
                // happened, and restart with startJob() using the new
                // url.
                info.reset();
                status = TS_Ready;
                startJob();
                return;
              }
          }

        // Throw an exception if the job failed
        if(info->isNonSuccess())
          throw std::runtime_error(info->getMessage());

        // Make sure all newly created files are added to the cache
        // index.
        for(int i=0; i<outFiles.size(); i++)
          owner->addToCache(hash, outFiles[i]);

        status = TS_Done;
      }
  }

  void setup(const Action *act, const Hash &h, ActionInstaller *_owner)
  {
    action = act;
    hash = h;
    owner = _owner;

    if(action->isCopy()) type = TT_FileCopy;
    else if(action->isRule())
      {
        const Rule *r = action->rule;
        if(r->type == RST_URL)
          {
            type = TT_Download;
            url = URLRule::get(r)->url;
          }
        else if(r->type == RST_Archive)
          {
            type = TT_Unpack;
            dir = ArcRule::get(r)->dir;
          }
        else assert(0);

        for(int i=0; i<r->deps.size(); i++)
          waitingFor[r->deps[i]] = NULL;
      }
    else assert(0);

    if(action->destlist.size())
      status = TS_Wanted;
    else
      status = TS_Unwanted;
  }

  typedef std::set<std::string> SSet;

  /* Set up outFiles and waitingFor, as well as moving status from
     TS_Wanted/TS_Unwanted to TS_Waiting or TS_Ready.
  */
  void fixPaths(TargetMap &tmap)
  {
    // Don't reprocess an already fixed target
    if(status != TS_Wanted &&
       status != TS_Unwanted)
      return;

    // Do we have outputs?
    if(status == TS_Wanted)
      {
        // Copy them over to outFiles
        const SSet &dest = action->destlist;
        outFiles.reserve(dest.size());
        SSet::const_iterator it;
        for(it = dest.begin(); it != dest.end(); it++)
          outFiles.push_back(*it);

      }
    else
      {
        assert(status == TS_Unwanted);

        // Set up a temporary output location, except for copy
        // operations
        if(type != TT_FileCopy)
          outFiles.push_back(owner->getTmpFile(hash));
      }

    if(outFiles.size())
      {
        // Pick the first file in the list for dependencies. It
        // doesn't matter which one we use, they all have the same
        // data.
        useForDeps = outFiles[0];
      }
    else
      {
        // For empty copy operations, we can use the source file directly
        assert(type == TT_FileCopy);
        useForDeps = action->source;
      }

    assert(useForDeps != "");

    // Do we have dependencies?
    if(waitingFor.size())
      {
        // Set our status to signal that we have unmet dependencies
        status = TS_Waiting;

        // Loop through and set up dependencies, then store their
        // output file names.
        HTMap::iterator it;
        for(it = waitingFor.begin(); it != waitingFor.end(); it++)
          {
            Target &targ = tmap[it->first];
            assert(it->second == NULL);
            it->second = &targ;

            // Set up the target.
            targ.fixPaths(tmap);
            assert(targ.status == TS_Ready ||
                   targ.status == TS_Waiting);
            assert(targ.useForDeps != "");
          }
      }
    else
      // This task is ready to be run directly
      status = TS_Ready;
  }
};

void ActionInstaller::doJob()
{
  ActionMap acts;
  TargetMap targets;

  getActions(acts);

  // Convert ActionMap to TargetMap
  {
    ActionMap::iterator it;
    for(it = acts.begin(); it != acts.end(); it++)
      targets[it->first].setup(&it->second, it->first, this);
  }

  // Set up output paths all targets
  TargetMap::iterator it;
  for(it = targets.begin(); it != targets.end(); it++)
    {
      Target &t = it->second;

      // Only process targets with that have output requests
      if(t.status == TS_Wanted)
        t.fixPaths(targets);
    }

  // Cull unwanted targets
  for(it = targets.begin(); it != targets.end();)
    {
      TargetMap::iterator it2 = it++;
      if(it2->second.status == TS_Unwanted)
        targets.erase(it2);
    }

  setBusy("Installing");

  /* Main loop. This updates and checks up on the status of all
     targets at regular intervals.
  */
  while(true)
    {
      if(checkStatus())
        // Abondon ship. Running jobs are aborted automatically by
        // Target's destructor.
        return;

      // Progress counters
      int64_t cur = 0, tot = 0;

      // Loop through and update all the targets
      bool allDone = true;
      for(it = targets.begin(); it != targets.end(); it++)
        {
          Target &t = it->second;

          t.update(cur, tot);

          if(t.status != TS_Done)
            allDone = false;
        }

      setProgress(cur,tot);

      if(allDone)
        break;

      // Don't busy-loop
      Thread::sleep(0.1);
    }

  setDone();
}
