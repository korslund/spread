#include "filejob.hpp"

using namespace Spread;

// Not sure this is right either
void FileJob::doJob()
{
  setBusy("Looking for " + inputHash.toString());
  outFile = fetchFile(inputHash, outFile);
  setDone();
}

std::string FileJob::fetchFile(const Hash &hash)
{
  MovableLock lock = owner.lock();
  bool onlyFile = false;
  JobInfoPtr lastInfo;

  /* Jump here to restart the search process. Spaghetti is nice once
     in a while :)
   */
 lookupFile:
  if(!finder.findHash(hash, src))
    throw std::runtime_error("No source for target " + hash.toString());

  // TODO: I guess we'll put back the target again? This is the only
  // place where it makes sense to check for TST_InPlace anyway, it's
  // pretty pointless to repeat this whole deal somewhere else. The
  // return below goes back to only applying if target == "".

  if(src.type == TST_File)
    {
      // If the file already exists, just return the location immediately
      assert(src.value != "");
      return src.value;
    }

  if(onlyFile)
    throw std::runtime_error("Failed to fetch " + hash.toString());

  assert(src.type == TST_Download ||
         src.type == TST_Archive);

  // Fetch any existing targets for this hash
  JobInfoPtr lastInfo = getInfo();
  bool found = owner.getTarget(hash, lastInfo);
  assert(lastInfo);

  // We no longer need to hold up the list
  if(lock) lock.reset();
  assert(!lock);

  if(found)
    {
      // There was an existing target. Wait for it to finish.
      assert(lastInfo != getInfo());

      // TODO: Make this an abortable wait. Make a wait(JobInfoPtr)
      // variant actually.
      lastInfo->wait();

      // Abort on error
      lastInfo->failError();

      // If the job succeeded, look up the file again, but restrict it
      // to cache lookup this time.
      onlyFile = true;
      goto lookupFile;
    }

  // There was NO existing target, and our own job has been inserted.
  assert(out == getInfo());

  // Set up a tmp target location
  std::string res = owner.getTmpName(hash);

  // Add the output
  output[res] = hash;

  /* OK let's START here: Define what the Target class should DO. How
     do you USE it, and what's the result. I think THIS is the
     problem.

     Not that strange, you've been dealing with this problem for HALF
     A YEAR. You already had one working solution. Now you're redoing
     it ExecJob style. And I KNOW there is a nice, tidy solution to
     this problem in here somewhere.
   */

  /* TODO: This is copied directly from Target::doJob(). Adjust it to
     fit into the above. URL failures have to restart the job, which
     means we have to do the restart: thing again. Allows us to detect
     a repeated URL btw, in case brokenURL failed for some reason.
     Just store a 'lastURL' so that we won't try that again.

     And obviously, calling fetchFile recursively will NOT work, since
     that doesn't add sub-jobs, it just reuses this object (which is
     BAD.) Instead, THAT is the point (in execHashTask below) where we
     should create a subjob and add it. No, not ONE subjob - LIST of
     subjobs! So THIS is also where we handle the LIST, and the
     archives! Yes, it all makes sense now.

     Another structural thing: This isn't where we need to return a
     string. The strings are passed through the cache now (much more
     sensible). The only place where we GET these strings are in the
     input thing below (now called fetchFile), which ideally should
     JUST check the cache, THEN create the new job if necessary, THEN
     wait for it and check the cache again. So the TOP part of the
     function above, basically.

     AARGH! This is a repeating tapestry, abcabcabcabcabc, and I don't
     know whether it's |abc|abc|abc|, |cab|cab|cab| or |bca|bca|bca|.

     PROBLEMS with the above:
     - well it doesn't auto-solve the archive stuff, but we'll do that
       later

     - it still creates sub-jobs for NON (wait, the last structural
       thing talks about it a little bit)
   */

  if(src.type == TST_File)
    execHashTask(maker.copyJob(src.value), true);
  else if(src.type == TST_Download)
    {
      if(!execHashTask(maker.downloadJob(src.value), false))
        {
          // Let the caller try to find a replacement
          owner->brokenURL(src.hash, src.value);
          throw std::runtime_error(lastJob->getMessage());
        }
    }
  else if(src.type == TST_Archive)
    {
      assert(src.dir);
      execHashTask(maker.unpackJob(*src.dir), true);
    }
  else assert(0);
  if(checkStatus()) return;

  /* The order is important here: we might have other threads waiting
     for us, thus we cannot set 'done' status until AFTER have added
     our output files to cache.

     notifyFiles() implementations must also make sure to add the
     files to cache before removing them from target lists used by
     other jobs.
   */
  assert(output.size());
  owner->notifyFiles(output);

  return res;
}

// These were from target.cpp. We can keep execHashTask as it is.

bool Target::execHashTask(HashTaskBase *htask, bool failOnError)
{
  assert(htask);
  assert(!src.hash.isNull());

  // Add inputs
  for(int i=0; i<src.deps.size(); i++)
    {
      const Hash &input = src.deps[i];
      assert(!input.isNull());
      assert(input != src.hash);
      htask->addInput(input, fetchFile(input));
    }

  // Add outputs
  Hash::DirMap::const_iterator it;
  for(it = output.begin(); it != output.end(); it++)
    htask->addOutput(it->second, it->first);

  return execJob(htask, failOnError);
}

void Target::doJob()
  setDone();
}
