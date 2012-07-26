#ifndef __SPREAD_RULE_HPP_
#define __SPREAD_RULE_HPP_

#include "hash/hash.hpp"
#include <vector>

namespace Spread
{
  struct Rule
  {
    std::string ruleString;
    std::vector<Hash> deps, outputs;

    void addDep(const Hash &h) { deps.push_back(h); }
    void addOut(const Hash &h) { outputs.push_back(h); }
  };
}

#endif
