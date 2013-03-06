#ifndef __SPREAD_RULEFINDER_HPP_
#define __SPREAD_RULEFINDER_HPP_

#include "hash/hash.hpp"
#include "rule.hpp"

namespace Spread
{
  typedef std::set<const Rule*> RuleList;

  struct RuleFinder
  {
    /* Return the best known Rule that produces the given hash. The
       returned Rule (if any) MUST contain 'hash' in its 'outputs'
       list.

       Returns NULL if no rule was found.
     */
    virtual const Rule *findRule(const Hash &hash) const = 0;

    /* Insert ALL known rules for the given hash into 'output'.
     */
    virtual void findAllRules(const Hash &hash, RuleList &output) const = 0;

    /* Report a broken URL back to the rule manager. MUST ensure that
       the URL is no longer returned by future calls to findRule() or
       findAllRules().
     */
    virtual void reportBrokenURL(const Hash &hash, const std::string &url) = 0;

    /* Get a list of archive hints for a given dirhash. The hashes in
       the list are of archives containing files relevant to the usage
       of the dirhash, possibly including the dir-object itself.

       Returns NULL if nothing was found.
     */
    virtual const std::vector<Hash>* findHints(const Hash &dirHash) const = 0;

    virtual ~RuleFinder() {}
  };
}

#endif
