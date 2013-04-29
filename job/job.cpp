#include "job.hpp"

#include <assert.h>
#include <exception>

using namespace Spread;

Job::Job() : info(new JobInfo)
{
  assert(info);
  info->status = ST_CREATED;
}

JobInfoPtr Job::run()
{
  assert(info->isCreated());
  assert(!info->isBusy());
  assert(!info->isFinished());
  if(info->doAbort)
    {
      info->status = ST_ABORT;
      return info;
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

  assert(info->isFinished());
  cleanup();
  return info;
}

bool Job::runClient(Job &job, bool includeStats, bool copyFail)
{
  assert(info->hasStarted());
  assert(!info->isFinished());
  if(clearClient(copyFail)) return true;
  if(includeStats) setClient(job.getInfo());
  else setAbortClient(job.getInfo());
  job.run();
  return clearClient(copyFail);
}

bool Job::runClient(JobPtr job, bool includeStats, bool copyFail)
{
  return runClient(*job.get(), includeStats, copyFail);
}

bool Job::waitClient(JobInfoPtr client, bool includeStats, bool copyFail)
{
  assert(client);
  if(clearClient(copyFail)) return true;
  if(includeStats) setClient(client);
  else setAbortClient(client);
  client->wait(getInfo());
  return clearClient(copyFail);
}

std::string Job::setBusy(const std::string &what)
{
  std::string old = info->message;
  info->message = what;
  info->status = ST_BUSY;
  return old;
}
