#include "jobholder.hpp"
#include <assert.h>
#include <job/thread.hpp>

#define LOCK boost::lock_guard<boost::mutex> lock(mutex)

using namespace Spread;

void JobHolder::clearFailed()
{
  LOCK;
  done.clear();
}

void JobHolder::doJob()
{
  startup();
  start();

  while(true)
    {
      /* Loop through jobs. We use special functions to ensure
         thread safety, since new jobs may be inserted at any time.
      */
      bool empty = false;
      {
        LOCK;
        JobSet::const_iterator it, it2;
        for(it = jobs.begin(); it != jobs.end();)
          {
            it2 = it++;

            // Simply remove all finished jobs
            if((*it2)->getInfo()->isFinished())
              {
                JobPtr p = *it2;
                jobs.erase(it2);

                // Keep failed jobs to help the user debug
                if(p->getInfo()->isError())
                  {
                    handleError(p->getInfo()->getMessage());
                    done.push_back(p);
                  }
              }
          }

        empty = jobs.size() == 0;
        if(finishOnEmpty && empty)
          setDone();
      }

      // This will exit if setDone() was called, or if the job was
      // aborted.
      if(checkStatus()) return;

      tick();

      Thread::sleep(0.1);

      // Sleep some more if there is nothing to do
      if(empty)
        Thread::sleep(0.5);
    }
}
