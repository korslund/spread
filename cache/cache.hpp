#ifndef __SPREAD_CACHE_HPP_
#define __SPREAD_CACHE_HPP_

#include "index.hpp"
#include "dir/directory.hpp"

/* Expand and test later.
 */

namespace Cache
{
  struct Cache
  {
    CacheIndex index;

    // Load a directory object
    Spread::DirectoryCPtr loadDir(const Spread::Hash &hash)
    {
      using namespace Spread;

      // We don't cache these in memory yet. For now, just find the
      // object through the index and load it, if it exists.
      const std::string &file = index.findHash(hash);
      DirectoryCPtr res;

      if(file != "")
        {
          Directory *dir = new Directory;
          Hash read = dir->read(file);
          assert(read == hash);
          res.reset(dir);
        }

      return res;
    }
  };
}

#endif
