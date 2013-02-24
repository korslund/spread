#ifndef __SPREAD_JOBHOLDER_HPP_
#define __SPREAD_JOBHOLDER_HPP_

#include "listjob.hpp"

/* JobHolder is a ListJob that holds a list of independently running
   thread jobs. New jobs are added with the add() functions (from
   ListJob). It is useful as a "top-level" interface object between a
   tree of running tasks, and the main application thread.

   Jobs are removed from the list when they succeed or are
   aborted. However failed jobs are kept in the 'done' list so the
   user can debug them.

   The JobHolder will instantly abort all its children if it is itself
   aborted.
*/

namespace Spread
{
  struct JobHolder : ListJob
  {
    // Called right after doJob() starts
    virtual void startup() {}

    // Called regularly by doJob() while the task runs
    virtual void tick() {}

    // When set to true, the job will finish when it runs out of jobs.
    bool finishOnEmpty;

    // Tell the job to terminate when all jobs have ended.
    void finish() { finishOnEmpty = true; }

    // Wait for all jobs to terminate
    void waitFinish() { finish(); info->wait(); }

    JobHolder() : finishOnEmpty(false) {}

  private:
    void doJob();
  };
}
#endif
