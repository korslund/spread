#ifndef __JOB_JOBINFO_HPP
#define __JOB_JOBINFO_HPP

#include <string>
#include <boost/smart_ptr.hpp>
#include <stdint.h>

namespace Spread
{
  enum JobStatus
    {
      ST_NONE,          // Job not yet created
      ST_CREATED,       // Job object created but not started
      ST_BUSY,          // Job is running
      ST_DONE,          // Job completed successfully
      ST_ERROR,         // Job failed
      ST_ABORT          // Job was aborted by user
    };

  struct Job;
  struct JobInfo;

  typedef boost::shared_ptr<JobInfo> JobInfoPtr;
  typedef boost::weak_ptr<JobInfo> JobInfoWPtr;

  /* Communication structure between a running job and the outside
     world.
   */
  struct JobInfo
  {
    JobInfo() { reset(); }

    int64_t getCurrent();
    int64_t getTotal();
    std::string getMessage();
    void abort();

    bool isCreated() const { return status >= ST_CREATED; }
    bool hasStarted() const { return status >= ST_BUSY; }
    bool isBusy() const { return status == ST_BUSY; }
    bool isSuccess() const { return status == ST_DONE; }
    bool isError() const { return status == ST_ERROR; }
    bool isAbort() const { return status == ST_ABORT; }
    bool isNonSuccess() const { return isError() || isAbort(); }
    bool isFinished() const { return isSuccess() || isNonSuccess(); }

    // These two may be used outside the Job class to signal external
    // status, if there is no running Job attached to this JobInfo
    // instance.
    void setDone() { status = ST_DONE; }
    void setError(const std::string &what);

    // Throw an exception on error or abort status
    void failError();

    void setProgress(int64_t cur, int64_t tot);
    void setProgress(int64_t cur);
    bool checkStatus();
    void reset();
    void setClient(JobInfoPtr client);
    void setStatsClient(JobInfoPtr client);
    void setAbortClient(JobInfoPtr client);
    void resetClient() { setClient(JobInfoPtr()); }
    bool clearClient(bool copyFail=true);
    void wait();

  private:

    friend struct Job;

    JobInfoWPtr statsClient, abortClient;

    // Progress. Only meant for informative purposes, not guaranteed
    // to be accurate.
    int64_t current, total;

    // See JobStatus
    int status;

    // Set to true if the outside world is requesting an abort
    bool doAbort;

    // Status or error message describing the current operation. Only
    // valid when isBusy() or isError() are true.
    std::string message;
  };
}

#endif
