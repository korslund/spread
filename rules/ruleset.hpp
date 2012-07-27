#ifndef __SPREAD_RULESET_HPP_
#define __SPREAD_RULESET_HPP_

#include "rulefinder.hpp"
#include <boost/shared_ptr.hpp>

/* RuleSet is an implementation of the abstract RuleFinder interface.

   The reason these are separated by abstraction is to improve
   testability and modularity, especially of the InstallFinder class.
 */

namespace Spread
{
  // These are used for the Rule::type field on all produced rules
  enum RuleSetTypes
    {
      RST_None          = 0,    // No/unknown rule type
      RST_URL           = 1,    // URL rule
      RST_Archive       = 2     // Archive / unpack rule
    };

  /* Specific Rule sublclasses for the rule types known to RuleSet.
   */
  struct URLRule : Rule
  {
    std::string url;
    bool isBroken;

    /* URL priorities are used to force some urls to always be chosen
       before others. If there exists more than one URL source for a
       given hash, the one with the highest priority is always
       returned. The only way a lower priority URL may be returned is
       if all the ones above it have failed (reported as broken.)

       This allows setting up fallback URLs to be used in case of
       temporary server failures, for example.

       If multiple sources share the highest priority, then weighting
       is used.
     */
    int priority;

    /* Weighting is used to pick from multiple sources that have the
       same priority. All the weights for the given priority level are
       summed, and each source is given a probability of weight/sum.
       Then a source is picked at random using those probabilities.
     */
    float weight;

    URLRule(const std::string &rulestr, const std::string &_url,
            int prio = 1, float wght = 1.0)
      : Rule(RST_URL, rulestr), url(_url), isBroken(false),
        priority(prio), weight(wght) {}
  };

  struct RuleSet : RuleFinder
  {
    RuleSet();

    /* Inherited from RuleFinder.

       NOTE: Returned pointers are only valid for the lifetime of the
       RuleSet instance. Objects are deleted when RuleSet destructs.
     */
    const Directory *findDir(const Hash &hash) const;
    const Rule *findRule(const Hash &hash) const;

    /* Add an URL rule.
    */
    void addURL(const Hash &hash, const std::string &url,
                int priority = 1, float weight = 1.0,
                std::string ruleString = "");

    /* Decode and add rule string.

       Known formats:

       "URL <hash> <url> [priority] [weight]"
     */
    void addRule(const std::string &ruleString) {} // Not implemented yet

    /* Report broken URL rule. This will remove the rule from all
       future rule searches.

       Later this will also invoke a callback (not implemented yet),
       which you can use to report the issue back to the server, or do
       other actions to correct the problem.
     */
    void reportBrokenURL(const Hash &hash, const std::string &url);

    // Get URLRule pointer from a Rule pointer
    const URLRule *getURL(const Rule *r);

  private:
    struct _RuleSetInternal;
    boost::shared_ptr<_RuleSetInternal> ptr;
  };

}

#endif
