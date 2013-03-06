#include "dir_install.hpp"
#include <install_jobs/hashfinder.hpp>
#include <rules/arcruleset.hpp>
#include <dir/tools.hpp>

using namespace Spread;

struct DirInstaller::_Internal
{
  RuleFinderPtr rulePtr;
  RuleSet *origRules;
  ArcRuleSet *arcRules;
  DirOwner *owner;
  std::string prefix;

  Hash::DirMap pre, post;
  TreeBase::HashDir preHash, postHash, preBlinds, postBlinds;
};

void DirInstaller::loadHints(const Hash &dirHash)
{
  log("Looking for hints for dirHash=" + dirHash.toString());

  /* TODO: This should of course allow the old style of hints as well,
     where an archive rule + dirobject exists and we can dump it
     directly into the ArcRuleSet.

     In fact two cases:

     - arcrule, dir exists: dump into arcruleset. Fetch system will
       download the archive and unpack files as necessary.

     - no arcrule or no dir: do a blind index (will auto-download
       archive if needed), and then dump the generated dir into
       arcruleset. Needs the ability to KNOW what dirhash was created
       from a blind job, we don't know that yet.
   */

  const std::vector<Hash> *hints = ptr->arcRules->findHints(dirHash);
  if(!hints || hints->size() == 0)
    {
      log("  No hints found.");
      return;
    }

  assert(0);

  for(int i=0; i<hints->size(); i++)
    {
      const Hash &hint = (*hints)[i];

      /* All hints given refer to archives for which there are no
         archive rules. We are only supposed to unpack them somewhere
         to make the files within accessible through the cache. The
         system will do the rest automatically.

         Do a blind unpack to a tmp location. The job will fetch the
         archive file itself if possible.

         TODO: NO, THIS IS BAD! Read the above note for a better
         solution.
      */
      assert(0);
      std::string where = owner.getTmpName(hint);
      TreePtr job = owner.unpackBlindTarget(where);
      job->addInput(hint);
      log("  Unpacking HINT=" + hint.toString() + " to " + where);
      execJob(job, false);
      if(checkStatus()) return;

      // Log errors and continue
      if(lastJob->isNonSuccess())
        {
          std::string errMsg = "  FAILED loding HINT=" + hint.toString() + "\n  Error: ";
          if(lastJob->isError()) errMsg += lastJob->getMessage();
          else errMsg += "Job aborted";
          errMsg += "\n  Attempting to continue without this hint.";
          log(errMsg);
        }
    }
}

DirPtr DirInstaller::addDirFile(Hash::DirMap &out, const Hash &dirHash,
                                const std::string &path)
{
  // Load any hints associated with the dir first
  loadHints(dirHash);

  // Fetch the dir file if possible
  std::string dirFile = fetchFile(dirHash);

  // Load and use it
  DirPtr dir(new Hash::DirMap);
  owner.loadDir(dirFile, *dir, dirHash);
  Dir::add(out, *dir, path);
  return dir;
}

void DirInstaller::handleHash(Hash::DirMap &out, const Hash &dirHash,
                              HashDir &blinds, const std::string &path)
{
  log("Determining what to do with input hash " + dirHash.toString());

  // Check if there is an archive associated with the given hash
  const ArcRuleData *arc = ptr->origRules->findArchive(dirHash);

  if(!arc)
    {
      /* Just load the dir and add it to 'out', along with any hints
         associated with it. We assume this is enough to find all the
         files we need. Throws on error.
       */
      addDirFile(out, dirHash, path);
      return;
    }

  /* We found an archive. Try loading the dirfile and injecting it
     into the ArcRuleSet. This will make the contained files directly
     available through the rule system, where the fetchFiles() system
     can find them. This is the preferred solution.
   */
  try
    {
      DirPtr arcDir = addDirFile(out, arc->dirHash, path);
      ptr->arcRules->addArchive(arc->arcHash, arc->dirHash, arcDir,
                                arc->ruleString);
    }
  catch(std::exception &e)
    {
      /* If the dirfile loading fails for whatever reason (perhaps the
         dirfile is not available anywhere), then fall back to doing a
         "blind" unpack.
       */
      log("FAILED loading dirHash=" + arc->dirHash.toString() +
          "\n   Error message: " + std::string(e.what()) +
          ".\n   Reverting to blind unpack");
      blinds.insert(HDValue(arc->dirHash, path));
    }
}

  /*
    So what happens on a first install, and on an upgrade?

    - virtually all games will have a single zip representing each
      hash. We don't need dir files in these cases. No dir file =
      blind install automatically. If a dir file is found (from
      earlier), then that works too of course.

    - upgrading: this is linked to the PRE package, not the post
      package (since that works blind.) So if a PRE hash is given, we
      are given an additional hint of where to download an upgrade
      zip. The pre dirhash should already be cached (but the below
      note means we have a fallback if it isn't.) Since that is
      cached, the hint associated with this OLD dir contains:

      - new dir file (required)
      - old dir file (not needed, but doesn't cost that much)
      - upgraded replacement files (if any)
      - patch files (optional)

      The ONLY thing we do is blind-unpack the hinted archive into
      a tmp dir. The rest is automatic!

    - in main doJob(): if we are upgrading (there exist pre items),
      and we are missing either dir file, then we have to do the
      blind-index thing mentioned above. Otherwise if we are NOT
      upgrading, do a direct blind install.

   */

void DirInstaller::sortInput()
{
  HashDir::const_iterator it;
  for(it = ptr->preHash.begin(); it != ptr->preHash.end(); it++)
    {
      handleHash(ptr->pre, it->first, ptr->preBlinds, it->second);
      if(checkStatus()) return;
    }
  for(it = ptr->postHash.begin(); it != ptr->postHash.end(); it++)
    {
      handleHash(ptr->post, it->first, ptr->postBlinds, it->second);
      if(checkStatus()) return;
    }
}

void DirInstaller::doJob()
{
  sortInput();
  if(checkStatus()) return;

  setDone();
}

DirInstaller::DirInstaller(DirOwner &owner, RuleSet &rules,
                           Cache::ICacheIndex &cache, const std::string &prefix)
  : TreeBase(owner)
{
  ptr.reset(new _Internal);
  ptr->origRules = &rules;
  ptr->arcRules = new ArcRuleSet(ptr->origRules);
  ptr->rulePtr.reset(ptr->arcRules);
  ptr->owner = &owner;
  ptr->prefix = prefix;

  finder.reset(new HashFinder(ptr->rulePtr, cache));
}

void DirInstaller::addFile(const std::string &file, const Hash &hash)
{
  assert(!getInfo()->hasStarted());
  ptr->pre[file] = hash;
}

void DirInstaller::remFile(const std::string &file, const Hash &hash)
{
  assert(!getInfo()->hasStarted());
  ptr->post[file] = hash;
}

void DirInstaller::addDir(const Hash::DirMap &dir, const std::string &path)
{
  assert(!getInfo()->hasStarted());
  Dir::add(ptr->pre, dir, path);
}

void DirInstaller::remDir(const Hash::DirMap &dir, const std::string &path)
{
  assert(!getInfo()->hasStarted());
  Dir::add(ptr->post, dir, path);
}

void DirInstaller::addDir(const Hash &hash, const std::string &path)
{
  assert(!getInfo()->hasStarted());
  ptr->preHash.insert(HDValue(hash,path));
}

void DirInstaller::remDir(const Hash &hash, const std::string &path)
{
  assert(!getInfo()->hasStarted());
  ptr->postHash.insert(HDValue(hash,path));
}
