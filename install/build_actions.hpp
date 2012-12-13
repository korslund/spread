#ifndef __SPREAD_BUILDACTIONS_HPP_
#define __SPREAD_BUILDACTIONS_HPP_

#include "rules/arcruleset.hpp"
#include "rules/ruleset.hpp"
#include "dir/directory.hpp"
#include "cache/cache.hpp"
#include "rules/actions.hpp"
#include <vector>

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
    void addDir(const Directory *dir, const std::string &path = "");
    void addDir(DirectoryCPtr dir, const std::string &path = "")
    { addDir(dir.get(), path); }

    /* Add a directory by hash. This assumes that either the directory
       is available as a loadable cached object somewhere, or that
       there is a matching archive rule for the directory.

       If alsoAsHint==true (default), then any archive rule that
       contains this directory is loaded, if it exists.

       If the directory object does not exist, but an archive rule is
       found, the directory is added as a "blind" unpack. That means
       that whatever the archive contains is used as the output
       directory.
     */
    void addDir(const Hash &hash, bool alsoAsHint = true, const std::string &path = "");

    /* Build the complete install action list from the given
       dependencies.
     */
    void build(ActionMap &output);

  private:
    Directory list;

    Cache::Cache &cache;
    RuleSet &rules;
    ArcRuleSet arcs;

    // List of blind archive unpacks
    std::vector<Hash> blind;

    // Path prefix, added to every filename in the list
    std::string prefix;
  };
}
#endif
