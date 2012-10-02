#include "action_installer.hpp"
#include "rules/urlrule.hpp"
#include "rules/arcrule.hpp"
#include "job/thread.hpp"
#include "htasks/unpackhash.hpp"
#include "htasks/downloadhash.hpp"
#include "htasks/copyhash.hpp"
#include <stdexcept>

using namespace Spread;

//#define DEBUG_PRINT
#ifdef DEBUG_PRINT
#include <iostream>
#define PRINT(a) std::cout << __LINE__ << ": " << a << "\n";
#else
#define PRINT(a)
#endif

/* TODO: testing: There isn't yet a complete and exhaustive test for
   this code. The current tests only test basic features. A complete
   test of the unpacking coordination and all the other features, with
   full output of all API functions called (file caching etc), for
   various predefined normal and pathological input cases, would be
   nice to have. Ideally it should also be possible to include
   -DDEBUG_PRINT when compiling this file only.

   Also, the Target struct could do well with some restructuring into
   base and subclasses. That would also make piecewise testing easier.
 */

enum Type
  {
    TT_FileCopy,
    TT_Download,
    TT_Unpack,
    TT_UnpackBlind,
    TT_Passive
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

  // Used for "blind" unpacks to store the output filenames and the
  // corresponding hashes. The paths stored are absolute paths.
  Directory::DirMap dirmap;

  int type, status;

  ActionInstaller *owner;

  // Set on archive targets to point to the target responsible for
  // unpacking.
  Target *unpacker;

  // Dependencies we are waiting for, and their final locations when
  // done.
  typedef std::map<Hash, Target*> HTMap;
  HTMap waitingFor;

  // Outputs that other targets want us to add to the final job. Used
  // for archive unpacking, where one target is responsible for
  // unpacking all the other targets.
  HTMap otherOutputs;

  JobInfoPtr info;

  // List of all output files. If there are no directly requested
  // outputs, but the target is implicitly required by other
  // targets, this list will contain a tmp file instead.
  std::vector<std::string> outFiles;

  // Targets that depend on us should use this file as their input
  // when we are TS_Done.
  std::string useForDeps;

  Target() : unpacker(NULL) {}
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

  // Check if all our dependencies are done. If so, move our status
  // from TS_Waiting to TS_Ready.
  void checkDeps()
  {
    assert(status == TS_Waiting);

    HTMap::iterator it;
    for(it = waitingFor.begin(); it != waitingFor.end(); it++)
      {
        const Target *t = it->second;
        assert(t);

        // If any of our dependencies are still busy, then we have to
        // wait for it to finish.
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
    status = TS_Running;

    PRINT("Starting job:");

    HashTask *htask;
    if(type == TT_FileCopy)
      {
        htask = new CopyHash(action->source);
        PRINT("  Copying " << action->source);
      }
    else if(type == TT_Download)
      {
        htask = new DownloadHash(url);
        PRINT("  Downloading " << url);
      }
    else if(type == TT_Unpack)
      {
        htask = new UnpackHash(dir->dir);
        PRINT("  Unpacking " << dir->dir.size() << " elements");
      }
    else if(type == TT_UnpackBlind)
      {
        assert(useForDeps != "");
        assert(hash.isNull());
        // The 'true' means 'return absolute paths'.
        htask = new UnpackHash(useForDeps, dirmap, true);
        PRINT("  Unpacking BLIND into " << useForDeps);
      }
    else if(type == TT_Passive)
      {
        /* Passive tasks are used when all our output files have
           already been created by our dependencies.

           An example is unpacking targets from an archive: one target
           is selected as the "unpacker" and does the actual work of
           creating ALL the targets, the rest are passive and simply
           wait for the unpacking target to finish.
         */
        PRINT("  Passive task (no job required)");
        return;
      }
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
          PRINT("  Input: " << h << " " << t.useForDeps);
        }
    }

    /*
      If we have no outputs, then we are done. This is only allowed
      for empty copy operations (which are basically just markers
      telling us about already existing files.)
    */
    if(outFiles.size() == 0)
      {
        assert(type == TT_FileCopy);
        status = TS_Done;
        PRINT("  File already exists");
        return;
      }

    // Add all our requests as task outputs.
    assert(outFiles.size());
    if(type != TT_UnpackBlind)
      for(int i=0; i<outFiles.size(); i++)
        {
          assert(!hash.isNull());
          htask->addOutput(hash, outFiles[i]);
          PRINT("  Output: " << hash << " " << outFiles[i]);
        }

    /* Add all outputs added by other targets. This is used for
       unpacking jobs.
     */
    if(otherOutputs.size())
      {
        /* Sanity check - we should only get here if we are the
           unpacker for an archive. You can remove this block later if
           you want to use otherOutputs for other things as well.
        */
        {
          assert(type == TT_Unpack);
          assert(waitingFor.size() == 1);
          Target *arc = waitingFor.begin()->second;
          assert(arc->unpacker == this);
        }

        HTMap::iterator it;
        for(it = otherOutputs.begin(); it != otherOutputs.end(); it++)
        {
          const Hash &h = it->first;
          const Target &t = *it->second;

          // For each target, add all the files in their outfile list
          for(int i=0; i<t.outFiles.size(); i++)
            {
              htask->addOutput(h, t.outFiles[i]);
              PRINT("  Ext Output: " << h << " " << t.outFiles[i]);
            }
        }
      }

    info = htask->getInfo();
    assert(info);
    Thread::run(htask);
  }

  void checkJob()
  {
    assert(status == TS_Running);
    assert(info || type == TT_Passive);

    bool finished = false;

    if(type == TT_Passive)
      {
        finished = true;
        assert(!info);
      }
    else if(info->isFinished())
      {
        PRINT("Job finished");

        // Check for recoverable errors
        if(info->isError() && type == TT_Download)
          {
            PRINT("  Download error: URL=" << url);
            // Try to get a replacement URL
            assert(url != "");
            std::string newUrl = owner->brokenURL(hash, url);
            if(newUrl != "")
              {
                // There was a replacement! Silently take one step
                // back to TS_Ready, pretend the botched job never
                // happened, and restart with startJob() using the new
                // url.
                info.reset();
                status = TS_Ready;
                url = newUrl;
                PRINT("  Found replacement URL: " << url);
                startJob();
                return;
              }
          }

        // Throw an exception if the job failed
        if(info->isNonSuccess())
          {
            std::string error = info->getMessage();
            if(type == TT_Download)
              error = "Error downloading " + url + "\nDetails:\n" + error;
            PRINT("  Throwing exception: " << error);
            throw std::runtime_error(error);
          }
        finished = true;
      }

    if(finished)
      {
        // Make sure all newly created files are added to the cache
        // index.
        if(type != TT_UnpackBlind)
          {
            // The "normal" case, with known output files
            for(int i=0; i<outFiles.size(); i++)
              owner->addToCache(hash, outFiles[i]);
          }
        else
          {
            // The "blind" archive case, where we depend on the info
            // from the unpack operation to know what files were
            // created.
            assert(dirmap.size());
            assert(useForDeps != "");
            Directory::DirMap::const_iterator it;
            for(it = dirmap.begin(); it != dirmap.end(); it++)
              {
                // The file paths created by UnpackHash in dirmap are
                // absolute paths, so we can use them directly.
                const std::string &file = it->first;
                const Hash &hsh = it->second;
                assert(!hsh.isNull());
                owner->addToCache(hsh, file);
              }
          }

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
            const ArcRule *arc = ArcRule::get(r);

            if(arc->dir)
              {
                dir = arc->dir;
                type = TT_Unpack;
              }
            else
              {
                /* Archive rules may be "blind", which means we don't
                   know up front what files are in it. This is the
                   case when the directory associated with the archive
                   could not be loaded. The default behavior then is
                   to unpack the archive as-is, and use the archive
                   itself as the directory.
                */
                type = TT_UnpackBlind;

                // Blind unpacks use dummy hashes. We don't need it,
                // so blank it out.
                assert(hash.size() == 0);
                hash.clear();
                assert(!dir);
              }
          }
        else assert(0);

        for(int i=0; i<r->deps.size(); i++)
          waitingFor[r->deps[i]] = NULL;
      }
    else assert(0);

    assert(type == TT_UnpackBlind || !hash.isNull());

    if(action->destlist.size())
      status = TS_Wanted;
    else
      status = TS_Unwanted;
  }

  typedef std::set<std::string> SSet;

  /* This sets up all inter-dependence relations between the targets.

     It set up outFiles and waitingFor, as well as moving status from
     TS_Wanted/TS_Unwanted to TS_Waiting or TS_Ready.

     Also sets up collective job control for targets that rely on
     one-to-many jobs, such as unpacking an archive file into many
     output files.
  */
  void fixDeps(TargetMap &tmap)
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
        assert(!hash.isNull());

        // Set up a temporary output location, except for copy
        // operations (where we can use the source file directly, see
        // below.)
        if(type != TT_FileCopy)
          outFiles.push_back(owner->getTmpFile(hash));
      }

    // Note that for blind archives (TT_UnpackBlind), the output list
    // will contain an output directory rather than a filename. Make
    // sure it's the only output.
    assert(type != TT_UnpackBlind || outFiles.size() == 1);

    if(outFiles.size())
      {
        // Pick the first file in the list for dependencies. It
        // doesn't matter which one we use, they all have the same
        // data. (For TT_UnpackBlind this is now the output
        // directory.)
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
            targ.fixDeps(tmap);
            assert(targ.status == TS_Ready ||
                   targ.status == TS_Waiting);
            assert(targ.useForDeps != "");
          }
      }
    else
      // This task is ready to be run directly
      status = TS_Ready;

    /* Are we an unpack operation? If so, we have to notify the target
       archive to coordinate our unpacking with the other targets.
     */
    if(type == TT_Unpack || type == TT_UnpackBlind)
      {
        // Get the archive target
        assert(waitingFor.size() == 1);
        Target *arc = waitingFor.begin()->second;
        assert(arc);

        if(arc->unpacker)
          {
            // Blind archives can only have one target
            assert(type != TT_UnpackBlind);

            // There is already an unpacker. Add ourselves as an
            // output.
            arc->unpacker->otherOutputs[hash] = this;

            // We now depend on the unpacker to do our work. We have
            // to wait for it to finish, but once it is we don't do
            // any work ourselves.
            waitingFor[arc->unpacker->hash] = arc->unpacker;
            type = TT_Passive;
          }
        else
          {
            // No unpacker has been selected. Set ourselves up for the
            // job.
            arc->unpacker = this;
          }
      }
  }
};

void ActionInstaller::doJob()
{
  ActionMap acts;
  TargetMap targets;

  PRINT("GETTING ACTIONS");

  getActions(acts);

  PRINT("Converting ActionMap => TargetMap");

  // Convert ActionMap to TargetMap
  {
    ActionMap::iterator it;
    for(it = acts.begin(); it != acts.end(); it++)
      targets[it->first].setup(&it->second, it->first, this);
  }

  PRINT("Solving dependencies");

  // Setup output paths and other inter-dependent information on all
  // targets
  TargetMap::iterator it;
  for(it = targets.begin(); it != targets.end(); it++)
    {
      Target &t = it->second;

      // Only process targets with that have output requests
      if(t.status == TS_Wanted)
        t.fixDeps(targets);
    }

  PRINT("Culling unused targets");

  // Cull unwanted targets
  for(it = targets.begin(); it != targets.end();)
    {
      TargetMap::iterator it2 = it++;
      if(it2->second.status == TS_Unwanted)
        targets.erase(it2);
    }

  setBusy("Installing");

  PRINT("STARTING INSTALL");

  /* Main loop. This updates and checks up on the status of all
     targets at regular intervals.
  */
  while(true)
    {
      PRINT("LOOP");

      if(checkStatus())
        // Abondon ship. Running jobs are aborted automatically by
        // Target's destructor.
        return;

      // Progress counters
      int64_t cur = 0, tot = 0;

      PRINT("UPDATING");

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

      PRINT("SLEEPING");

      // Don't busy-loop
      Thread::sleep(0.1);

      PRINT("END LOOP\n");
    }

  setDone();
}
