#ifndef __SPREAD_RULE_HPP_
#define __SPREAD_RULE_HPP_

#include "hash/hash.hpp"
#include <vector>

namespace Spread
{
  // These values are used for the Rule::type field
  enum RuleSetTypes
    {
      RST_None          = 0,    // No/unknown rule type
      RST_URL           = 1,    // URL rule
      RST_Archive       = 2     // Archive / unpack rule
    };

  struct Rule
  {
    // Rule type and description.
    int type;
    std::string ruleString;

    // List of inputs (dependencies) and outputs (products) of this
    // rule.
    std::vector<Hash> deps, outputs;

    Rule(int tp=0, const std::string &str = "")
      : type(tp), ruleString(str) {}
    virtual ~Rule() {}

    void addDep(const Hash &h) { deps.push_back(h); }
    void addOut(const Hash &h) { outputs.push_back(h); }
  };
}

#endif
