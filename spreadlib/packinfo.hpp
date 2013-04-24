#ifndef __SPREADLIB_PACKINFO_HPP_
#define __SPREADLIB_PACKINFO_HPP_

#include <string>
#include <vector>
#include <stdint.h>
#include <assert.h>

namespace Spread
{
  typedef std::vector<std::string> StrVec;

  struct PackInfo
  {
    std::string channel, package, version;
    StrVec dirs, paths;
    uint64_t installSize;

    PackInfo() : installSize(0) {}

    bool match(const PackInfo &other) const
    {
      if(other.dirs.size() != dirs.size()) return false;
      assert(dirs.size() == paths.size());
      assert(dirs.size() == other.paths.size());
      for(int i=0; i<dirs.size(); i++)
        if(dirs[i] != other.dirs[i] || paths[i] != other.paths[i])
          return false;
      return true;
    }
  };

  typedef std::vector<PackInfo> PackInfoList;
};

#endif
