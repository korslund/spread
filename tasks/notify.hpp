#ifndef __TASKS_NOTIFY_HPP_
#define __TASKS_NOTIFY_HPP_

#include <job/job.hpp>

namespace Tasks
{
  /* NotifyTask is meant to be subclassed, and is used for getting
     notifications when another task succeeds or fails.

     Pass along a job to the constructor, and run it as normal. The
     destructor deletes the job.
   */
  struct NotifyTask : Jobs::Job
  {
    NotifyTask(Jobs::Job *j);
    ~NotifyTask() { delete other; }

  protected:
    // One of these are called when the job has finished
    virtual void onSuccess() {}
    virtual void onError() {}

  private:
    Jobs::Job *other;

    void doJob();
    void cleanup();
  };
}

#endif
