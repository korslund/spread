#include "target.hpp"
#include <stdexcept>

using namespace Spread;

std::string Target::fetchFile(const Hash &hash)
{
  JobPtr fetchTask;
  std::string output = owner->fetchFile(hash, fetchTask);
  assert(output != "");

  /* If there is a job required to fetch this file, then fetchFile()
     will have set it up for us. Execute it.
  */
  if(fetchTask)
    execJob(fetchTask, true);

  return output;
}

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
{
  assert(owner);
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
  setDone();
}
