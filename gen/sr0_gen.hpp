#ifndef __SPREAD_GEN_SR0GEN_HPP_
#define __SPREAD_GEN_SR0GEN_HPP_

#include "cache/cache.hpp"
#include "rules/ruleset.hpp"

namespace SpreadGen
{
  struct GenSR0
  {
    GenSR0(Cache::Cache &_cache,
           const Spread::RuleSet &_rules)
      : cache(_cache), rules(_rules) {}

    /* Create an SR0 output set. Consists of:

       short.txt - short version of output directory hash
       _output - directory to be zipped and uploaded as index.zip

       The input is an archive file that contains the full output
       directory. We assume that the RuleSet contains an ARC rule for
       the archive, and that the directory object is loadable through
       the Cache.
     */
    void makeSR0(const std::string &arcFile, const std::string &outDir);

    /*
      Version that takes an archive hash or dir hash as parameter,
      instead of an archive file.
     */
    void makeSR0(const Spread::Hash &hash, const std::string &outDir);

  private:
    Cache::Cache &cache;
    const Spread::RuleSet &rules;
  };
}

#endif
