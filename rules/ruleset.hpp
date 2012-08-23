#ifndef __SPREAD_RULESET_HPP_
#define __SPREAD_RULESET_HPP_

#include "rulefinder.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

/* RuleSet is an implementation of the abstract RuleFinder interface.

   The RuleSet holds the global part of the rule set in memory. It
   holds URL and archive rules, but does not index hashes within the
   archives themselves.
 */

namespace Spread
{
  struct ArcRuleData
  {
    Hash arcHash, dirHash;
    std::string ruleString;

    ArcRuleData(const Hash &arc, const Hash &dir,
                const std::string &str)
      : arcHash(arc), dirHash(dir), ruleString(str) {}
  };

  struct RuleSet : RuleFinder
  {
    RuleSet();

    /* Inherited from RuleFinder.

       NOTE: Returned pointers are only valid for the lifetime of this
       RuleSet instance. Objects are deleted when RuleSet destructs.
     */
    const Rule *findRule(const Hash &hash) const;

    /* Inherited from RuleFinder.

       Find all rules corresponding to a hash and add them to the
       given set.
     */
    void findAllRules(const Hash &hash, RuleList &output) const;

    /* Add an URL rule. URL rules are directly searchable through
       findRule().
    */
    void addURL(const Hash &hash, const std::string &url,
                int priority = 1, float weight = 1.0,
                std::string ruleString = "");

    /* Add an archive rule. These are searchable through
       findArchive().
     */
    void addArchive(const Hash &hash, const Hash &dirHash,
                    std::string ruleString = "");

    /* Search archive database. Will return archives that matches
       EITHER the archive hash or the dir hash.

       Returns NULL if nothing is found.
     */
    const ArcRuleData *findArchive(const Hash &hash) const;

    /* Report broken URL rule. This will remove the rule from all
       future rule searches.

       This will also invoke a callback (if set through
       setURLCallback) which you can use to report the issue back to
       the server, or do other actions to correct the problem.
     */
    void reportBrokenURL(const Hash &hash, const std::string &url);

    // Set callback to handle broken URLs
    typedef boost::function< void(const Hash &hash, const std::string &url) > CBFunc;
    void setURLCallback(CBFunc cb) { callback = cb; }

  private:
    struct _RuleSetInternal;
    boost::shared_ptr<_RuleSetInternal> ptr;
    CBFunc callback;
  };

}

#endif
