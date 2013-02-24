#ifndef __SPREAD_LIST_JOB_HPP_
#define __SPREAD_LIST_JOB_HPP_

#include "parentjob.hpp"

/* ListJob is a ParentJob that has multiple child jobs, each running
   in a separate thread.

   New jobs can be added with add() before or after the job has
   started. The list is thread safe, meaning you can safely add new
   jobs from external threads.

   This is an abstract class. For usable implementations see AndJob
   and JobHolder.
 */

namespace Spread
{
  struct ListJob : ParentJob
  {
    /* Add a new child job to the list. The job must be freshly
       created (not started yet). The job is started by start(), or
       immediately if start() has already been executed.
     */
    void add(JobPtr j);

    // Convenience pointer version. Only use on pointers NOT owned by
    // a shared_ptr / JobPtr.
    void add(Job *j) { add(JobPtr(j)); }

    ListJob() : started(false) {}

  protected:
    /* Call start() from doJob() whenever you are ready to start the
       jobs. This will start all jobs already added to the list, and
       will also make add() auto-start any jobs added from this point
       on.
     */
    void start();

  private:
    bool started;
  };
}
#endif
