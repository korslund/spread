#ifndef __SPREAD_INSTALL_BASEJOB_HPP_
#define __SPREAD_INSTALL_BASEJOB_HPP_

#include "treebase.hpp"

namespace Spread
{
  // TODO: Just make this part of ijobmaker instead.

  struct TargetBase : TreeBase
  {
    TargetBase(blah) : TreeBase(blah) {}

    virtual void addOutput(const Hash &h, const std::string &file = "") = 0;
  };

  typedef boost::shared_ptr<TargetBase> TargetPtr;
}

#endif
