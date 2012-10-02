#ifndef __SPREAD_ACTIONS_HPP_
#define __SPREAD_ACTIONS_HPP_

#include "rule.hpp"
#include <set>
#include <map>
#include <boost/shared_ptr.hpp>

namespace Spread
{
  /* The Action struct is used in the output result from
     InstallFinder.

     It represents an action required to produce a set of output
     files, either by copying an existing file or by generating the
     data from a rule.

     Each Action represents one Hash and thus one unique data content
     which the input and output must match. If the Action has multiple
     output files listed, then all the created files will be identical
     copies of the same data.
  */
  struct Action
  {
    // Source file to copy from. Only valid if rule == NULL
    std::string source;

    /* List of target files. The action should create all these
       files. Output actions with empty target lists can safely be
       ignored, unless other actions depend on them.

       NOTE: A special case for blind archive rules: the dest list
       will contain an output directory instead of a filename.
    */
    std::set<std::string> destlist;

    /* Rule used to perform this action, if not NULL. Also of
       importance is the rule->deps fields, which lists hash of
       objects that must be in place BEFORE this rule can be acted
       upon.
    */
    const Rule *rule;

    /* Returns true if this Action does not represent any valid
       action, ie. there were no rules or sources found for this
       target.
    */
    bool isNone() const { return rule == NULL && source == ""; }

    // True if this is a copy action, from 'source' to 'destlist'.
    bool isCopy() const { return rule == NULL && source != ""; }

    // True if this is a Rule action, using 'rule' to create
    // 'destlist'.
    bool isRule() const { return rule != NULL && source == ""; }

    void addDest(const std::string &dest)
    { if(dest != "") destlist.insert(dest); }

    Action() : rule(NULL) {}
    Action(const std::string &src, const std::string &dest = "")
      : source(src), rule(NULL) { addDest(dest); }
    Action(const Rule *r, const std::string &dest = "")
      : rule(r) { addDest(dest); }
  };

  typedef std::map<Hash,Action> ActionMap;
}
#endif
