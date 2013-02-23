#ifndef __JOB_THREAD_HPP
#define __JOB_THREAD_HPP

#include "job.hpp"

namespace Spread
{
  /* This black-box module swallows any Job object, executes it in a
     separate thread, then deletes the object after it returns. (The
     job must have been allocated using 'new', and cannot be owned by
     a JobPtr.)

     You can disable threading (run it locally) by setting
     async=false. This may seem pointless compared to just running
     j->run(), but it does provide a consistent interface for both
     cases, and it will also delete the Job before returning.

     The JobPtr based version does not explicitly delete the job, just
     resets the smart_ptr to it. Use this version if you want the
     object to survive after running.
   */
  struct Thread
  {
    static JobInfoPtr run(Job *j, bool async=true);
    static JobInfoPtr run(JobPtr j, bool async=true);
    static void sleep(double seconds);
  };
}
#endif
