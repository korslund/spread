#ifndef __SPREAD_URLRULE_HPP_
#define __SPREAD_URLRULE_HPP_

#include "rule.hpp"
#include <assert.h>

namespace Spread
{
  // URL rule
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

    URLRule(const Hash &hash, const std::string &rulestr,
            const std::string &_url, int prio = 1, float wght = 1.0)
      : Rule(RST_URL, rulestr), url(_url), isBroken(false),
        priority(prio), weight(wght) { addOut(hash); }

    // Get URLRule pointer from a Rule pointer
    static const URLRule *get(const Rule *r)
    {
      assert(r->type == RST_URL);
      const URLRule *ur = dynamic_cast<const URLRule*>(r);
      assert(ur != NULL);
      return ur;
    }
  };
}
#endif
