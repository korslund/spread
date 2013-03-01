#include "filejob.hpp"

using namespace Spread;

std::string FileJob::fetchFile(const Hash &hash, JobPtr &job, const std::string &target)
{
  TargetPtr t(new Target(this, maker));

  /* Lock the target list to avoid race conditions. Possible scenario:

     Targets are added to cache AFTER we call findHash, but removed
     from the list BEFORE we call getTarget(). That would make us miss
     the targets, even though they exists. The result would be a
     non-catastrofic but expensive regeneration (eg. a redownload) of
     an existing file.

     Locking prevents targets from being removed, which means
     getTarget is guaranteed to find them if they exist.
   */
  MovableLock lock = owner.lock();

  // It's nice to have spaghetti once in a while :)
  bool onlyFile = false;
 lookupFile:

  if(!finder.findHash(hash, t->src, target))
    throw std::runtime_error("No source for target " + hash.toString() + " " + target);

  if(t->src.type == TST_InPlace)
    {
      assert(target == t->src.value);
      return target;
    }

  if(t->src.type == TST_File)
    {
      // If there is a file and no target, just return the file
      // location directly
      if(target == "")
        {
          assert(t->src.value != "");
          return t->src.value;
        }

      // Otherwise, set up a copy operation
      t->output[target] = hash;
      job = t;
      return target;
    }

  if(onlyFile)
    throw std::runtime_error("Failed to fetch " + hash.toString() + " " + target);

  assert(t->src.type == TST_Download ||
         t->src.type == TST_Archive);

  // Fetch any existing targets for this hash
  JobPtr out = t;
  bool found = owner.getTarget(hash, out);
  // We no longer need to hold up the list
  if(lock) lock.reset();

  if(found)
    {
      // There was an existing target. Wait for it to finish.
      assert(out && out != t);
      assert(!lock);
      out->getInfo()->wait();

      // The file should now be in the cache. Go look it up again, and
      // fail if we can't find it.
      onlyFile = true;
      goto lookupFile;
    }

  // There was NO existing target, and our own job has been inserted.
  assert(out == t);

  // Set up a tmp target location if necessary
  std::string res = target;
  if(res == "")
    res = owner.getTmpName(hash);

  // Add the output to the job
  t->output[res] = hash;

  // Return our job to the caller
  job = t;

  return res;
}

void FileJob::doJob()
{
  assert(0);
  // TODO: If a job fails and we can't find a replacement for it, fail
  // with the error message of the last job we executed. You'll have
  // to try-catch fetchTmpFile() for it, but that's ok.

  // Store the result in outFile.
}
