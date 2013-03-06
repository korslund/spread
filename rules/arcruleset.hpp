#ifndef __SPREAD_ARCRULESET_HPP_
#define __SPREAD_ARCRULESET_HPP_

#include "rulefinder.hpp"
#include <dir/ptr.hpp>
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
    /* If set to non-NULL, all calls to findRule() and findAllRules()
       will include matches found 'base' as well as in the ArcRuleSet
       itself.

       Calls to reportBrokenURL() and findHints() are passed through
       to base unfiltered.
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
    void addArchive(const Hash &arcHash, const Hash &dirHash,
                    DirCPtr dir, const std::string &ruleString = "");

    // These are simply deferred to base (if set)
    const std::vector<Hash>* findHints(const Hash &dirHash) const
    { if(base) return base->findHints(dirHash); return NULL; }
    void reportBrokenURL(const Hash &hash, const std::string &url)
    { if(base) base->reportBrokenURL(hash, url); }


  private:
    RuleFinder *base;
    struct _Internal;
    boost::shared_ptr<_Internal> ptr;
  };
}

#endif
