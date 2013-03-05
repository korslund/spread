#ifndef __SPREAD_HASHFINDER_HPP_
#define __SPREAD_HASHFINDER_HPP_

#include "ihashfinder.hpp"
#include <rules/rulefinder.hpp>
#include <cache/iindex.hpp>

namespace Spread
{
  typedef boost::shared_ptr<RuleFinder> RuleFinderPtr;

  struct HashFinder : IHashFinder
  {
    RuleFinderPtr rules;
    Cache::ICacheIndex &cache;

    HashFinder(RuleFinderPtr r, Cache::ICacheIndex &c)
      : rules(r), cache(c) { assert(r); }

    bool findHash(const Hash &hash, HashSource &out, const std::string &target="");

    void brokenURL(const Hash &hash, const std::string &url)
    { rules->reportBrokenURL(hash, url); }

    void addToCache(const Hash::DirMap &files)
    { cache.addMany(files); }
  };
}

#endif
