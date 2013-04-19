#include "packlister.hpp"
#include "dir/binary.hpp"

#include <assert.h>
#include <stdexcept>

using namespace SpreadGen;
using namespace Spread;

Hash PackLister::addDir(const std::string &packName, const Hash &hash,
                        const std::string &path)
{
  Hash dirHash = process(hash);

  // Find the dir object
  const std::string dirFile = cache.findHash(dirHash);
  if(dirFile == "")
    {
      std::string error = "Pack '" + packName + "': could not load directory hash "
        + dirHash.toString();

      if(dirHash != hash)
        error += " (original hash: " + hash.toString() + ")";

      throw std::runtime_error(error);
    }

  Hash::DirMap dir;
  {
    Hash dirHashRead = Dir::read(dir, dirFile);
    if(dirHashRead != dirHash)
      {
        std::string error =
          "Dir file mismatch in " + dirFile + ": wanted " + dirHash.toString() +
          ", got " + dirHashRead.toString() + " (package " + packName + ")";

        if(dirHash != hash)
          error += " (original hash: " + hash.toString() + ")";

        throw std::runtime_error(error);
      }
  }

  uint64_t bytes = 0;

  // Loop through all the files in the output dir
  Hash::DirMap::const_iterator it;
  for(it = dir.begin(); it != dir.end(); it++)
    {
      // Add any additional rules we can find to resolve each file
      const Hash &target = it->second;
      rules.findAllRules(target, ruleSet);

      // Add up file sizes
      bytes += target.size();
    }

  PackInfo &pinf = packs[packName];
  pinf.dirs.push_back(dirHash.toString());
  pinf.paths.push_back(path);
  pinf.package = packName;
  assert(pinf.dirs.size() == pinf.paths.size());

  /* Add sizes. TODO: This is technically wrong, because it double-
     counts overwritten files. However since overwrites are rare in
     our current data we can fix it later. The installSize is a
     cosmetic value in any case.
   */
  pinf.installSize += bytes;

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
