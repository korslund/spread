#ifndef __SPREAD_JOBS_LEAF_FACTORY_HPP__
#define __SPREAD_JOBS_LEAF_FACTORY_HPP__

#include "treebase.hpp"

namespace Spread
{
  struct LeafFactory
  {
    static TreePtr copyTarget(TreeOwner &owner, const std::string &from);
    static TreePtr downloadTarget(TreeOwner &owner, const std::string &url);
    static TreePtr unpackTarget(TreeOwner &owner, const Hash &dir);
    static TreePtr unpackBlindTarget(TreeOwner &owner, const std::string &where,
                                     const Hash &dir = Hash());
  };
}

#endif
