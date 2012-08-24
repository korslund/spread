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
       be applied in the order given.
     */
    void addDir(const std::string &packName,
                const Spread::Hash &dirHash);

    /* Add a hint to a package. Hints are used to provide archives
       that can help us fulfill the dependencies added with addDir(),
       but which do not themselves contribute to the actual output
       directory.

       For example, you could have multiple independent output
       directories added through addDir(), but one big archive
       containing all the actual files. Or conversely, many small
       archives covering up one output dir. Adding the archives as a
       hints will let Spread resolve all the dependencies correctly.

       Throws exception on error.

       TODO: We intend to automate the hint generation in a separate
       layer later on, or possibly replacing it entirely.

       IOW, the system will find all possible sources of all files
       automatically, at gen time, then optimize the dataset so that
       the minimal amount of data needs to be downloaded at runtime
       (but non- optimal paths are included as fallback in case of
       download errors etc.)

       This is all complicated stuff and isn't a high priority at the
       moment.
     */
    void addHint(const std::string &packName,
                 const Spread::Hash &dirHash);

    // Output:

    typedef std::vector<Spread::Hash> HashList;
    typedef std::set<Spread::Hash> HashSet;

    struct PackInfo
    {
      HashList dirs, hints;
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
