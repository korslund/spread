#ifndef __JOB_JOB_HPP
#define __JOB_JOB_HPP

#include "jobinfo.hpp"

namespace Spread
{
  struct Job
  {
    /* The constructor creates a new JobInfo struct and sets it to
       ST_CREATED status. You can then use getInfo() to retrieve a
       pointer to the struct.

       The JobInfo is owned by boost::shared_ptr, and may survive even
       after the Job object has been deleted.
     */
    Job();
    virtual ~Job() {}

    JobInfoPtr run();
    JobInfoPtr getInfo() { return info; }

  protected:
    /* Run the actual job. Overwrite this in child classes.
     */
    virtual void doJob() = 0;

    // Do optional exit handling. Called both on normal returns and
    // exceptions. Check 'info' for error status
    virtual void cleanup() {}

    JobInfoPtr info;

    // Check current status. If this returns true, doJob() should
    // immediately exit.
    bool checkStatus() { return info->checkStatus(); }

    /* Finish client job, and check status. If this returns true,
       doJob() should exit immediately.

       This implicitly calls checkStatus() as well, so you don't need
       to check both.
    */
    bool clearClient() { return info->clearClient(); }

    /* Run a client job in the current thread. Calls setClient,
       job.run() and clearClient(). Returns the result of
       clearClient().

       Exit doJob() immediately if the function returns true.
     */
    bool runClient(Job &job, bool includeStats=true);

    /* Same as runClient, but instead waits for a job running in a
       background thread. This is useful when you don't have access to
       the client's Job directly, only the info ptr.

       Like the above functions, you should exit doJob() immediately
       if the function returns true.
     */
    bool waitClient(JobInfoPtr client, bool includeStats=true);

    void setClient(JobInfoPtr inf) { info->setClient(inf); }
    void setStatsClient(JobInfoPtr inf) { info->setStatsClient(inf); }
    void setAbortClient(JobInfoPtr inf) { info->setAbortClient(inf); }
    void setProgress(int64_t cur, int64_t tot) { info->setProgress(cur, tot); }
    void setProgress(int64_t cur) { info->setProgress(cur); }

    void setBusy(const std::string &what = "");
    void setDone() { info->setDone(); }
    void setError(const std::string &what) { info->setError(what); }
  };
}

#endif
