#ifndef __SPREAD_AND_JOB_HPP_
#define __SPREAD_AND_JOB_HPP_

#include "listjob.hpp"

/* AndJob is a ListJob where all the child jobs have to succeed before
   the AndJob itself is considered successful.

   The job succeeds ONLY IF job1 AND job2 AND job3 AND ... are all
   successful, hence the name AndJob.

   If any of the jobs returns with an error or abort status, then the
   AndJob itself fails. In the case of failure, all active child jobs
   are immediately aborted. This is also true if the AndJob itself is
   sent an abort signal.

   AndJob sums the current and total progress numbers of all the child
   jobs.
 */

namespace Spread
{
  struct AndJob : ListJob
  {
    // All public functions are in base classes

  private:
    void doJob();
  };
}
#endif
