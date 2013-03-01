#ifndef __SPREAD_IJOBMAKER_HPP_
#define __SPREAD_IJOBMAKER_HPP_

#include <htasks/hashtaskbase.hpp>

/* Abstract interface for JobMaker, the class responsible for creating
   the end-level worker jobs. Abstracting this makes the job system
   much easier to test without using real zips and URLs.
 */

namespace Spread
{
  struct IJobMaker
  {
    virtual HashTaskBase* copyJob(const std::string &from) = 0;
    virtual HashTaskBase* downloadJob(const std::string &url) = 0;
    virtual HashTaskBase* unpackJob(const Hash::DirMap &index) = 0;
  };
}

#endif
