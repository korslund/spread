#ifndef __SPREAD_RULEFINDER_HPP_
#define __SPREAD_RULEFINDER_HPP_

#include "hash/hash.hpp"
#include "rule.hpp"

namespace Spread
{
  struct RuleFinder
  {
    /* Return the best known Rule that produces the given hash. The
       returned Rule (if any) MUST contain 'hash' in its 'outputs'
       list.

       Returns NULL if no rule was found.
     */
    virtual const Rule *findRule(const Hash &hash) const = 0;
    virtual ~RuleFinder() {}
  };
}

#endif
