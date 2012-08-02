#ifndef __TASKS_MULTI_HPP_
#define __TASKS_MULTI_HPP_

#include <job/job.hpp>
#include <list>

namespace Tasks
{
  /* A MultiTask performs several jobs in sequence.

     Tasks are added with add(), and all tasks must be added before
     the MultiTask has started running.

     Sub-tasks are executed one after another in the order they were
     added. They are all executed directly in the same thread as
     doJob() is running; there is no internal awareness of
     multithreading.

     If one sub-task fails, the entire task fails. The task only
     succeeds when all the sub-tasks have succeeded.

     Job objects are deleted when the multitask is deleted.

     Progress reports (through info->getCurrent() and getTotal()) will
     work, and aborting tasks will abort both the current running job
     as well as the whole sequence. Progress numbers only represent
     the currently running task, and will reset between tasks (so task
     1 will go from 0% to 100%, then task 2 will start back at 0% and
     up to 100%, etc.)
   */

  struct MultiTask : Jobs::Job
  {
    /* Add a job. If useInfo=false, we do NOT use the jobInfo of this
       task for progress statistics. This is useful if you're adding a
       small cleanup task after the main task, and don't want it to
       reset the progress reports (info->getCurrent/Total()) from the
       main task.

       useInfo only affects the progress part of the JobInfo struct.
       Aborts will still be passed through to the client job.

       Calling add() gives the MultiTask ownership of the Job
       instance. It is not safe to access the pointer after calling
       add().
    */
    void add(Jobs::Job *j, bool useInfo=true)
    { joblist.push_back(JPair(JobPtr(j),useInfo)); }

  protected:
    void doJob();

  private:
    typedef boost::shared_ptr<Jobs::Job> JobPtr;
    typedef std::pair<JobPtr, bool> JPair;
    typedef std::list<JPair> JList;
    JList joblist;
  };
}

#endif
