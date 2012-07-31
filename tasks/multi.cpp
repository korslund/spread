#include "multi.hpp"

using namespace Tasks;
using namespace Jobify;

MultiTask::MultiTask(JobInfoPtr inf)
  : Job(inf?inf:makeInfo())
{ assert(info); }

MultiTask::~MultiTask()
{
  JList::iterator it;
  for(it = joblist.begin(); it != joblist.end(); ++it)
    delete it->first;
}

/* Internal JobInfo that has a client pointer. It fetches update
   information from the client, and passes on abort() requests to it.

   The main purpose of this setup is to shield our main JobInfo (the
   MultiInfo) from the 'DONE' status of individual tasks. As each task
   is completed, we don't want the outside world to mistakenly think
   that WE are done, when we are not.
 */
struct MultiInfo : JobInfo
{
  Jobify::JobInfoPtr abortInfo, statsInfo;

  void abort()
  {
    doAbort = true;
    if(abortInfo) abortInfo->abort();
  }

  int64_t getCurrent()
  {
    if(statsInfo) current = statsInfo->getCurrent();
    return current;
  }

  int64_t getTotal()
  {
    if(statsInfo) total = statsInfo->getTotal();
    return total;
  }
};

JobInfoPtr MultiTask::makeInfo()
{
  return JobInfoPtr(new MultiInfo());
}

void MultiTask::doJob()
{
  assert(info);
  MultiInfo *inf = dynamic_cast<MultiInfo*>(info.get());

  // Run sub tasks
  JList::iterator it;
  for(it = joblist.begin(); it != joblist.end(); ++it)
    {
      Job &j = *it->first;
      bool useInfo = it->second;
      JobInfoPtr client = j.getInfo();
      if(inf)
        {
          inf->abortInfo = client;
          if(useInfo)
            inf->statsInfo = client;
        }
      j.run();

      // Abort on failure
      if(!client->isSuccess())
      {
        if(client->isError())
          setError(client->message);

        return;
      }

      // Check abort status
      if(checkStatus())
        return;
    }

  // Everything is OK
  setDone();
}
