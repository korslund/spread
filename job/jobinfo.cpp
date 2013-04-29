#include "jobinfo.hpp"
#include <assert.h>
#include <stdexcept>
#include "thread.hpp"

using namespace Spread;

int64_t JobInfo::getCurrent()
{
  JobInfoPtr p = statsClient.lock();
  if(p) current = p->getCurrent();
  return current;
}

int64_t JobInfo::getTotal()
{
  JobInfoPtr p = statsClient.lock();
  if(p) total = p->getTotal();
  return total;
}

std::string JobInfo::getMessage()
{
  JobInfoPtr p = statsClient.lock();
  if(p) message = p->getMessage();
  return message;
}

void JobInfo::abort()
{
  JobInfoPtr p = abortClient.lock();
  if(p) p->abort();
  doAbort = true;
}

void JobInfo::failError()
{
  checkStatus();
  if(isError())
    throw std::runtime_error(getMessage());
  else if(isAbort())
    throw std::runtime_error("Job aborted (last status: '" + getMessage() + "')");
}

void JobInfo::setClient(JobInfoPtr client)
{
  setStatsClient(client);
  setAbortClient(client);
}

void JobInfo::setStatsClient(JobInfoPtr client)
{
  assert(!client || client.get() != this);
  statsClient = client;
}

void JobInfo::setAbortClient(JobInfoPtr client)
{
  assert(!client || client.get() != this);
  abortClient = client;
}

void JobInfo::wait(JobInfoPtr inf)
{
  while(!isFinished())
    {
      if(inf && inf->checkStatus()) break;
      Thread::sleep(0.05);
    }
}

bool JobInfo::clearClient(bool copyFail)
{
  JobInfoPtr p = abortClient.lock();
  abortClient.reset();
  if(p)
    {
      // Copy error states from the client info structs
      if(copyFail && p->isNonSuccess())
        {
          status = p->status;
          message = p->getMessage();
        }
    }
  p = statsClient.lock();
  statsClient.reset();
  if(p)
    {
      // Copy error states from the client info structs
      if(copyFail && p->isNonSuccess())
        status = p->status;

      // Copy final progress as well
      current = p->getCurrent();
      total = p->getTotal();
      message = p->getMessage();
    }
  return checkStatus();
}

void JobInfo::setProgress(int64_t cur, int64_t tot)
{
  current = cur;
  total = tot;
}

void JobInfo::setProgress(int64_t cur)
{
  current = cur;
}

void JobInfo::reset()
{
  statsClient.reset();
  abortClient.reset();
  current = total = 0;
  status = ST_NONE;
  doAbort = false;
  message = "";
}

void JobInfo::setError(const std::string &what)
{
  message = what;

  /* Don't override abort messages - we don't want to potentially
     trigger error messages to the user when they have already
     requested aborting the job.

     If you call info->abort(), you expect isAbort() to be true, not
     isError().
   */
  checkStatus();
  if(status != ST_ABORT)
    status = ST_ERROR;
}

bool JobInfo::checkStatus()
{
  // Check for abort requests
  if(doAbort) status = ST_ABORT;

  // Return true if any non-busy state has been set
  return !isBusy();
}
