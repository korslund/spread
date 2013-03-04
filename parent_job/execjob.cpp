#include "execjob.hpp"
#include <assert.h>
#include <stdexcept>

using namespace Spread;

bool ExecJob::execJob(JobPtr p, bool failOnError)
{
  assert(!jobs.size());
  jobs.insert(p);

  lastJob = p->getInfo();
  runClient(p, true, false);
  jobs.erase(p);
  done.push_back(p);

  assert(lastJob);
  if(failOnError) lastJob->failError();
  return lastJob->isSuccess();
}
