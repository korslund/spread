#include "jobinfo.hpp"
#include <assert.h>

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

void JobInfo::setClient(JobInfoPtr client)
{
  setStatsClient(client);
  setAbortClient(client);
}

void JobInfo::setStatsClient(JobInfoPtr client)
{
  assert(client.get() != this);
  statsClient = client;
}

void JobInfo::setAbortClient(JobInfoPtr client)
{
  assert(client.get() != this);
  abortClient = client;
}

bool JobInfo::clearClient()
{
  JobInfoPtr p = abortClient.lock();
  abortClient.reset();
  if(p)
    {
      // Copy error states from the client info structs
      if(p->isNonSuccess())
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
      if(p->isNonSuccess())
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
  status = ST_ERROR;
}

bool JobInfo::checkStatus()
{
  // Check for abort requests
  if(doAbort) status = ST_ABORT;

  // Return true if any non-busy state has been set
  return !isBusy();
}
