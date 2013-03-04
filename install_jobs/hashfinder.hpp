#ifndef __SPREAD_HASHFINDER_HPP_
#define __SPREAD_HASHFINDER_HPP_

#include "ihashfinder.hpp"
#include <rules/rulefinder.hpp>
#include <cache/iindex.hpp>

namespace Spread
{
  struct HashFinder : IHashFinder
  {
    RuleFinder &rules;
    Cache::ICacheIndex &cache;

    HashFinder(RuleFinder &r, Cache::ICacheIndex &c)
      : rules(r), cache(c) {}

    bool findHash(const Hash &hash, HashSource &out, const std::string &target="");

    void brokenURL(const Hash &hash, const std::string &url)
    { rules.reportBrokenURL(hash, url); }

    void addToCache(const Hash::DirMap &files)
    { cache.addMany(files); }
  };
}

#endif
