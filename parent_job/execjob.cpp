#include "execjob.hpp"
#include <assert.h>
#include <stdexcept>

using namespace Spread;

bool ExecJob::execJob(JobPtr p, bool failOnError)
{
  assert(getInfo()->hasStarted());
  assert(!getInfo()->isFinished());
  assert(!jobs.size());

  jobs.insert(p);
  lastJob = p->getInfo();
  runClient(p, true, false);
  jobs.erase(p);
  done.push_back(p);

  assert(lastJob);
  assert(lastJob->isFinished());
  if(failOnError) lastJob->failError();
  return lastJob->isSuccess();
}
