#ifndef __SPREADLIB_PACKSTATUS_HPP_
#define __SPREADLIB_PACKSTATUS_HPP_

#include "packinfo.hpp"
#include <list>

namespace Spread
{
  /* Record of an installed package in the filesystem.
   */
  struct PackStatus
  {
    // Pack info at the time of install
    PackInfo info;

    // Installation path (must be absolute)
    std::string where;

    // True if the latest PackInfo no longer matches our 'info'
    bool needsUpdate;
  };

  typedef std::list<const PackStatus*> PackStatusList;
};

#endif
