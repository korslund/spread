#include "packlister.hpp"

#include <assert.h>
#include <stdexcept>

using namespace SpreadGen;
using namespace Spread;

void PackLister::addHint(const std::string &packName, const Hash &dirHash)
{
  Hash dir = process(dirHash);
  packs[packName].hints.push_back(dir);
}

Hash PackLister::addDir(const std::string &packName, const Hash &hash)
{
  Hash dirHash = process(hash);

  DirectoryCPtr dir = cache.loadDir(dirHash);
  if(!dir)
    {
      std::string error = "Pack '" + packName + "': could not load directory hash "
        + dirHash.toString();

      if(dirHash != hash)
        error += " (original hash: " + hash.toString() + ")";

      throw std::runtime_error(error);
    }

  // Loop through all the files in the output dir
  Directory::DirMap::const_iterator it;
  for(it = dir->dir.begin(); it != dir->dir.end(); it++)
    {
      // Add any additional rules we can find to resolve each file
      const Hash &target = it->second;
      rules.findAllRules(target, ruleSet);
    }

  packs[packName].dirs.push_back(dirHash);

  return dirHash;
}

Hash PackLister::process(const Hash &hash)
{
  Hash dirHash = hash;

  // Check for archives matching the hash
  const ArcRuleData *arc = rules.findArchive(hash);

  if(arc)
    {
      // Make sure we have the dir hash, and not the archive hash
      assert(hash == arc->dirHash || hash == arc->arcHash);
      dirHash = arc->dirHash;

      // List the archive into the output data
      arcSet.insert(arc);

      // Find and add rules that provides the archive itself
      rules.findAllRules(arc->arcHash, ruleSet);
    }

  // Add the directory file to the output dir list
  dirs.insert(dirHash);

  return dirHash;
}
