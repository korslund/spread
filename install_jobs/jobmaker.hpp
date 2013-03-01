#ifndef __SPREAD_JOBMAKER_HPP_
#define __SPREAD_JOBMAKER_HPP_

#include "ijobmaker.hpp"

namespace Spread
{
  struct JobMaker : IJobMaker
  {
    HashTaskBase* copyJob(const std::string &from);
    HashTaskBase* downloadJob(const std::string &url);
    HashTaskBase* unpackJob(const Hash::DirMap &index);
  };
}

#endif
