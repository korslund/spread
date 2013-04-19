#include "rule_writer.hpp"
#include "misc/readjson.hpp"
#include <algorithm>

using namespace std;
using namespace Spread;
using namespace SpreadGen;

/* Used to sort a list of strings before appending it to a Json
   array. The vector is sorted in place.
 */
static void appendSorted(Json::Value &out, vector<string> &input)
{
  assert(out.isNull() || out.isArray());

  sort(input.begin(), input.end());

  int old = out.size();
  out.resize(old + input.size());
  for(int i=0; i<input.size(); i++)
    out[old+i] = input[i];
}

// We could expose this function later, but right now we don't need it
// exposed, so keep it static.
static Json::Value rulesToJson(const RuleList &rules, const ArcRuleSet &arcs)
{
  Json::Value out;

  {
    RuleList::const_iterator it;
    vector<string> tmp;
    tmp.reserve(rules.size());
    for(it = rules.begin(); it != rules.end(); it++)
      tmp.push_back((*it)->ruleString);

    /* Sort the rules before adding them. Otherwise they will be
       ordered by pointer order (since RuleList is a set<> of
       pointers.) This would make output non-deterministic, leading to
       unnecessary hash changes, uploads and refreshes.
    */
    appendSorted(out, tmp);
  }
  {
    set<const ArcRuleData*>::const_iterator it;
    vector<string> tmp;
    tmp.reserve(arcs.size());
    for(it = arcs.begin(); it != arcs.end(); it++)
      tmp.push_back((*it)->ruleString);

    appendSorted(out, tmp);
  }

  return out;
}

void SpreadGen::writeRulesJson(const RuleList &rules,
                            const ArcRuleSet &arcs,
                            Mangle::Stream::StreamPtr out)
{
  ReadJson::writeJson(out, rulesToJson(rules, arcs));
}
