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

    /* Add a "blind" archive dependency. This means that we do not
       know the exact content of an archive at this point, we just
       want it unpacked into the given location.

       Using this saves you the trouble of keeping and distributing
       directory files for all the archives.
     */
    void addBlind(const std::string &path, const Hash &arcHash)
    { blinds.push_back(DepPair(path, arcHash)); }

    /* Perform the rule resolution. Takes the list of input
       dependencies in 'deps' and converts it to a list of actions in
       'output'.

       Returns true on success, or false if the output contains unmet
       dependencies.
     */
    bool perform(ActionMap &output) { return handleDeps(deps, output, true); }

    typedef std::pair<std::string,Hash> DepPair;
    typedef std::vector<DepPair> DepList;

    /* This is the struct's input data. You can set it up through the
       addDep/addBlind functions above, or manually if you prefer.
       Then run perform() to produce the output.

       Running perform() does not change the depslists, so the struct
       is reusable.
     */
    DepList deps, blinds;

  private:
    const RuleFinder &rules;
    Cache::CacheIndex &cache;
    bool handleDeps(const DepList &deps, ActionMap &output, bool baseLevel=false);
  };
}
#endif
