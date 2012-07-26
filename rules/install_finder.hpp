#ifndef __SPREAD_INSTALL_FINDER_HPP_
#define __SPREAD_INSTALL_FINDER_HPP_

#include "dir/directory.hpp"
#include "cache/index.hpp"
#include "hash/hash.hpp"
#include "rulefinder.hpp"
#include <vector>
#include <set>
#include <map>

namespace Spread
{
  struct InstallFinder
  {
    InstallFinder(const RuleFinder &rules, Cache::CacheIndex &cache);

    /* Add a directory of dependencies. Dependencies are the list of
       files to be installed. The directory can either contain full
       paths, or paths relative to reldir.

       Dependencies may overwrite the same location. It's always the
       depdency that's added last that takes precedence. This can be
       used intentionally to install eg. a game, and then a patch on
       top of it.
     */
    void addDep(const Directory &deps, const std::string &reldir = "");

    /* Add a dependency by looking up a directory rule. The files
       names in the directory are added relative to reldir.
    */
    void addDep(const Hash &hash, const std::string &reldir);

    // Add a single dependency. Must be a full path name
    void addDep(const std::string &file, const Hash &hash)
    { deps.push_back(DepPair(file,hash)); }

    /* Perform the rule resolution. Takes the list of input
       dependencies in 'deps' and converts it to a list of actions in
       'output'.
     */
    void perform() { handleDeps(deps); }

    struct Action
    {
      std::string action;
      std::set<std::string> destlist;
      const Rule *rule;

      void addDest(const std::string &dest)
      { if(dest != "") destlist.insert(dest); }

      Action() : rule(NULL) {}
      Action(const std::string &act, const std::string &dest = "", const Rule *r = NULL)
        : action(act), rule(r) { addDest(dest); }
    };

    typedef std::pair<std::string,Hash> DepPair;
    typedef std::vector<DepPair> DepList;
    typedef std::map<Hash,Action> ActionMap;

    /* This is the struct's input data. You can set it up through the
       addDep functions above, or manually if you prefer. Then run
       perform() to produce the output.

       Running perform() does not change the depslist. So to repeat
       the process you can just .clear() the output and run it again.
     */
    DepList deps;

    /* This is the output produced by perform().
     */
    ActionMap output;

  private:
    const RuleFinder &rules;
    Cache::CacheIndex &cache;

    void handleDeps(const DepList &deps);
  };
}
#endif
