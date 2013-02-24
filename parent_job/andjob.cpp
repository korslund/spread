#include "andjob.hpp"
#include <assert.h>
#include <job/thread.hpp>

#define LOCK boost::lock_guard<boost::mutex> lock(mutex)

using namespace Spread;

void AndJob::doJob()
{
  start();

  // Regularly check up on the jobs
  while(true)
    {
      int64_t current = 0, total = 0;

      bool hasBusy = false;

      {
        JobSet::const_iterator it;
        LOCK;
        for(it = jobs.begin(); it != jobs.end(); it++)
          {
            const JobPtr &j = *it;
            assert(j);
            JobInfoPtr inf = j->getInfo();

            current += inf->getCurrent();
            total += inf->getTotal();

            if(!inf->isFinished())
              hasBusy = true;
            else
              if(inf->isNonSuccess())
                {
                  setError("One or more child jobs did not succeed");
                  return;
                }
          }
      }

      setProgress(current, total);

      // Exit if all jobs have finished, or if there was an error
      if(!hasBusy || checkStatus())
        break;

      Thread::sleep(0.1);
    }

  // Move completed jobs over to 'done'
  JobSet::const_iterator it;
  LOCK;
  for(it = jobs.begin(); it != jobs.end(); it++)
    {
      const JobPtr &j = *it;
      assert(j);
      assert(j->getInfo()->isSuccess());
      done.push_back(j);
    }
  jobs.clear();

  if(checkStatus()) return;
  setDone();
}
