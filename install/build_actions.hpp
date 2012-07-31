#ifndef __SPREAD_BUILDACTIONS_HPP_
#define __SPREAD_BUILDACTIONS_HPP_

#include "rules/arcruleset.hpp"
#include "rules/ruleset.hpp"
#include "dir/directory.hpp"
#include "cache/cache.hpp"
#include "rules/actions.hpp"

/* ActionBuilder produces an ActionMap list of actions from a
   requested list of output files or directories.

   The produced ActionMap contains all the necessary information to
   complete an installation based on existing rules and data. It can
   be sent to ActionInstaller to perform the actual installation.
 */

namespace Spread
{
  struct ActionBuilder
  {
    ActionBuilder(Cache::Cache &_cache, RuleSet &_rules,
                  const std::string &_prefix = "")
      : cache(_cache), rules(_rules), arcs(&rules), prefix(_prefix)
    {}

    // Add archive hint. The hash can be either for the archive file
    // itself or for the directory of its contents.
    void addHint(const Hash &hint);

    // Add a single dependency. The file path is taken to be relative
    // to 'prefix'.
    void addDep(const std::string &file, const Hash &hash);

    // Add a directory of dependencies
    void addDir(const Directory *dir);
    void addDir(DirectoryCPtr dir) { addDir(dir.get()); }

    /* Add a directory by hash. This assumes the directory is
       available as a loadable cached object somewhere.

       If alsoAsHint==true (default), then any archive rule that
       contains this directory is also loaded.
     */
    void addDir(const Hash &hash, bool alsoAsHint = true);

    /* Build the complete install action list from the given
       dependencies.
     */
    void build(ActionMap &output);

  private:
    Directory list;

    Cache::Cache &cache;
    RuleSet &rules;
    ArcRuleSet arcs;

    // Path prefix, added to every filename in the list
    std::string prefix;
  };
}
#endif
