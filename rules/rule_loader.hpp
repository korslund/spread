#ifndef __RULES_JSON_LOADER_HPP_
#define __RULES_JSON_LOADER_HPP_

#include <json/json.h>

namespace Spread
{
  struct RuleSet;

  /* Add rules from various sources. You may stack multiple calls on
     top of each other, the rules are simply added to the RuleSet in
     sequence.

     The return value (where present) is the number of rules added.
  */

  // Load rules from a json value
  extern int loadRulesJson(RuleSet&, const Json::Value&);
  // Load from a json file
  extern int loadRulesJsonFile(RuleSet&, const std::string &file);

  // Add one rule from a rule string
  extern void addRule(RuleSet&, const std::string &ruleString);
}

#endif
