#ifndef __SPREADLIB_PACKINFO_HPP_
#define __SPREADLIB_PACKINFO_HPP_

#include <string>
#include <vector>
#include <stdint.h>

namespace Spread
{
  typedef std::vector<std::string> StrVec;

  struct PackInfo
  {
    std::string channel, package, version;
    StrVec dirs, paths;
    uint64_t installSize;
  };

  typedef std::vector<PackInfo> PackInfoList;
};

#endif
