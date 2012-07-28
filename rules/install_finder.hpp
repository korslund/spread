#ifndef __SPREAD_INSTALL_FINDER_HPP_
#define __SPREAD_INSTALL_FINDER_HPP_

#include "cache/index.hpp"
#include "hash/hash.hpp"
#include "rulefinder.hpp"
#include "actions.hpp"
#include <vector>

namespace Spread
{
  struct InstallFinder
  {
    InstallFinder(const RuleFinder &rules, Cache::CacheIndex &cache);

    /* Add a dependency. A dependency is a request for a specific
       object 'hash' to be installed in location 'file'.

       Several dependencies may overwrite the same location. It's
       always the one added last that takes effect. This can be used
       intentionally to install eg. a game, and then a patch on top of
       it.
     */
    void addDep(const std::string &file, const Hash &hash)
    { deps.push_back(DepPair(file,hash)); }

    /* Perform the rule resolution. Takes the list of input
       dependencies in 'deps' and converts it to a list of actions in
       'output'.
     */
    void perform(ActionMap &output) { handleDeps(deps, output); }

    typedef std::pair<std::string,Hash> DepPair;
    typedef std::vector<DepPair> DepList;

    /* This is the struct's input data. You can set it up through the
       addDep functions above, or manually if you prefer. Then run
       perform() to produce the output.

       Running perform() does not change the depslist, so the struct
       is reusable.
     */
    DepList deps;

  private:
    const RuleFinder &rules;
    Cache::CacheIndex &cache;

    void handleDeps(const DepList &deps, ActionMap &output);
  };
}
#endif
