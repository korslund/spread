#ifndef __SPREAD_EXEC_JOB_HPP_
#define __SPREAD_EXEC_JOB_HPP_

#include "parentjob.hpp"

/* ExecJob is a ParentJob that has at most one active child at any
   given time.

   Rather than running the child in a new thread, the child is
   executed in the same thread as the ExecJob itself.
 */

namespace Spread
{
  struct ExecJob : ParentJob
  {
  protected:
    JobInfoPtr lastJob;

    /* Execute 'p' as a child of this job. The job is executed
       in-thread. The child lists are set up accordingly, making the
       child visible to the outside. Throws on error.

       If failOnError=false, child jobs are allowed to fail. On
       non-success, execJob() will then return false. You may inspect
       lastJob for details.
     */
    bool execJob(JobPtr p, bool failOnError=true);

    /* Convenience pointer version. Must be called with a newly
       created object that is not yet owned by a shared_ptr.
    */
    bool execJob(Job *p, bool failOnError=true)
    { return execJob(JobPtr(p), failOnError); }
  };
}
#endif
