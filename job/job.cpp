#include "job.hpp"

#include <assert.h>
#include <exception>

using namespace Jobs;

Job::Job() : info(new JobInfo)
{
  assert(info);
  info->status = ST_CREATED;
}

void Job::run()
{
  assert(info->isCreated());
  assert(!info->isBusy());
  assert(!info->isFinished());
  if(info->doAbort)
    {
      info->status = ST_ABORT;
      return;
    }
  setBusy();

  // Make sure we capture all exceptions, and flag them as job
  // failures.
  try
    { doJob(); }
  catch(std::exception &e)
    { setError(e.what()); }
  catch(...)
    { setError("Unknown error"); }

  assert(!info->isBusy());
  cleanup();
}

bool Job::runClient(Job &job, bool includeStats)
{
  if(clearClient()) return true;
  if(includeStats) setClient(job.getInfo());
  else setAbortClient(job.getInfo());
  job.run();
  return clearClient();
}

void Job::setBusy(const std::string &what)
{
  info->message = what;
  info->status = ST_BUSY;
}
