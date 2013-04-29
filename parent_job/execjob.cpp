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

  /* This handles aborts as well. If we are aborted during execution,
     then lastJob is aborted as well. This causes failError() to throw
     an exception. However, since aborts take precedence over errors
     in our job subsystem, the result will be isAbort() status, not
     isError() status.
   */
  if(failOnError)
    lastJob->failError();
  return lastJob->isSuccess();
}
