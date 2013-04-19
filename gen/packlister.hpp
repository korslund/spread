#ifndef __SPREAD_GEN_PACKLISTER_HPP_
#define __SPREAD_GEN_PACKLISTER_HPP_

#include "cache/iindex.hpp"
#include "rules/ruleset.hpp"
#include "spreadlib/packinfo.hpp"
#include <vector>
#include <set>

/* The PackLister builds a list of files, rules and other information
   needed to be included in a dataset, based on a given input list of
   packages.
 */

namespace SpreadGen
{
  typedef std::set<const Spread::ArcRuleData*> ArcRuleSet;

  struct PackLister
  {
    PackLister(Cache::ICacheIndex &_cache,
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

    typedef std::set<Spread::Hash> HashSet;

    /* Package list. Note that the 'channel' member is left blank
       because it is not used by ListWriter. The channel name is not
       included anywhere in the written output and is in fact not
       defined as a concept within the channel data itself. Also,
       installSize is not computed.
     */
    std::map<std::string, Spread::PackInfo> packs;

    // Directory files to include in the output
    HashSet dirs;

    // Rules to include
    Spread::RuleList ruleSet;

    // Archive rules to include
    ArcRuleSet arcSet;

  private:
    Cache::ICacheIndex &cache;
    const Spread::RuleSet &rules;

    // Process a directory or archive hash. Used by both the add*()
    // functions above. Returns the dir hash.
    Spread::Hash process(const Spread::Hash &hash);
  };
}

#endif
