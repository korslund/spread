#include "job.hpp"

#include <assert.h>
#include <exception>
#include "thread.cpp"

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

bool Job::runClient(Job &job, bool includeStats)
{
  if(clearClient()) return true;
  if(includeStats) setClient(job.getInfo());
  else setAbortClient(job.getInfo());
  job.run();
  return clearClient();
}

bool Job::waitClient(JobInfoPtr client, bool includeStats)
{
  if(clearClient()) return true;
  if(clearClient()) return true;
  if(includeStats) setClient(client);
  else setAbortClient(client);
  while(!client->isFinished()) Thread::sleep(0.05);
  return clearClient();
}

void Job::setBusy(const std::string &what)
{
  info->message = what;
  info->status = ST_BUSY;
}
