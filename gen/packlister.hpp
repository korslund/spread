#ifndef __SPREAD_GEN_PACKLISTER_HPP_
#define __SPREAD_GEN_PACKLISTER_HPP_

#include "cache/cache.hpp"
#include "rules/ruleset.hpp"
#include <vector>
#include <set>

/* The PackLister builds a list of files, rules and other information
   needed to be included in a dataset, based on a given input list of
   packages.
 */

namespace SpreadGen
{
  struct PackLister
  {
    PackLister(Cache::Cache &_cache,
               const Spread::RuleSet &_rules)
      : cache(_cache), rules(_rules) {}

    /* Add an output directory (represented by the directory hash, or
       a matching archive hash) to a destination package. Assumes the
       directory is findable and loadable through the cache.

       You may add multiple directories to the same output. They will
       be applied in the order given. You can also specify an optional
       prefix to add to all the names in the dir (make sure to
       slash-terminate it if you want to create subdirectories!)

       Returns the dir hash (which may differ from dirHash if dirHash
       represented an archive file.)
     */
    Spread::Hash addDir(const std::string &packName,
                        const Spread::Hash &dirHash,
                        const std::string &path = "");

    // Set (optional) version string describing this package release
    void setVersion(const std::string &packName, const std::string &version)
    { packs[packName].version = version; }

    // Output:

    typedef std::vector<Spread::Hash> HashList;
    typedef std::vector<std::string> StrList;
    typedef std::set<Spread::Hash> HashSet;

    struct PackInfo
    {
      HashList dirs;
      StrList paths;
      std::string version;
    };

    // Package list
    std::map<std::string, PackInfo> packs;

    // Directory files to include in the output
    HashSet dirs;

    // Rules to include
    Spread::RuleList ruleSet;

    // Archive rules to include
    std::set<const Spread::ArcRuleData*> arcSet;

  private:
    Cache::Cache &cache;
    const Spread::RuleSet &rules;

    // Process a directory or archive hash. Used by both the add*()
    // functions above. Returns the dir hash.
    Spread::Hash process(const Spread::Hash &hash);
  };
}

#endif
