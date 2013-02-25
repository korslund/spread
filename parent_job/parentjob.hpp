#ifndef __SPREAD_PARENT_JOB_HPP_
#define __SPREAD_PARENT_JOB_HPP_

#include <job/job.hpp>
#include <boost/thread/mutex.hpp>
#include <set>
#include <vector>

/* ParentJob: base class for jobs that have children.

   The list of children (split between a set of active jobs and a
   vector of completed jobs) is designed to be inspectable by external
   code.

   For thread safety it is required that you lock 'mutex' while
   accessing the lists.
 */

namespace Spread
{
  struct ParentJob : Job
  {
    typedef std::set<JobPtr> JobSet;
    typedef std::vector<JobPtr> JobVec;

    // Lock this whenever you are manipulating or reading either of
    // the lists.
    boost::mutex mutex;

    // Remember to lock the mutex while reading these!
    const JobSet &getJobs() { return jobs; }
    const JobVec &getDone() { return done; }

  protected:
    // Set of active jobs
    JobSet jobs;

    // List of finished jobs, in order of completion.
    std::vector<JobPtr> done;

  private:
    // Aborts all active jobs on exit.
    void cleanup();
  };
}
#endif
