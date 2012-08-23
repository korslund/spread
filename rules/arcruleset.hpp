#ifndef __SPREAD_ARCRULESET_HPP_
#define __SPREAD_ARCRULESET_HPP_

#include "rulefinder.hpp"
#include "dir/directory.hpp"
#include <boost/shared_ptr.hpp>

/* ArcRuleSet is a filter on top of another RuleFinder, that adds the
   option of searching for objects inside individual archives. Only
   archives specifically added (or "hinted") to ArcRuleSet are
   searched.

   Rationale:

   The staticly loaded RuleSet does not have a complete lookup of all
   individual files in all archives, because this data set is
   potentially huge. Therefore it is more practical to use a temporary
   ArcRuleSet on top of a RuleSet, and "prime" it with the archives we
   think are likely to contain the information we need. Then once the
   install is complete, the data can be discarded.
 */

namespace Spread
{
  struct ArcRuleSet : RuleFinder
  {
    /* If base is specified, all failed searches in findRule() is
       redirected to base. Otherwise, failed searches always return
       NULL.
     */
    ArcRuleSet(RuleFinder *_base = NULL);

    /* Search for a rule, includes individual files in preloaded
       archives in the search. If nothing is found, we defer to
       base->findRule(), which may return NULL.

       Objects returned should be considered owned by the ArcRuleSet,
       thus pointers are only valid for the lifetime of this instance.
     */
    const Rule *findRule(const Hash &hash) const;

    /* Find all rules for a given hash. In addition to inserting our
       own archive rules (if any), this also calls
       base->findAllRules().

       NOTE: At the moment we only store one archive rule per hash, so
       this will never insert more than one rule (not counting
       whatever base->findAllRules inserts.)
     */
    void findAllRules(const Hash &hash, RuleList &output) const;

    /* Associate a given archive with files in a directory. The
       necessary data can be obtained from RuleSet::findArchive().
     */
    void addArchive(const Hash arcHash, DirectoryCPtr dir,
                    const std::string &ruleString = "");

  private:
    RuleFinder *base;
    struct _Internal;
    boost::shared_ptr<_Internal> ptr;
  };
}

#endif
