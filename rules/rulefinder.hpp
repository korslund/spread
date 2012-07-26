#ifndef __SPREAD_RULEFINDER_HPP_
#define __SPREAD_RULEFINDER_HPP_

#include "hash/hash.hpp"
#include "dir/directory.hpp"
#include "rule.hpp"

namespace Spread
{
  struct RuleFinder
  {
    virtual const Directory *findDir(const Hash &hash) const = 0;
    virtual const Rule *findRule(const Hash &hash) const = 0;
  };
}

#endif
