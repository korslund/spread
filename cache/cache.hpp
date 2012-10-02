#ifndef __SPREAD_CACHE_HPP_
#define __SPREAD_CACHE_HPP_

#include "index.hpp"
#include "dir/directory.hpp"
#include "misc/random.hpp"

#include <iostream>
/* Expand and test later.
 */

namespace Cache
{
  struct Cache
  {
    CacheIndex index;
    Misc::Random random;

    std::string tmpDir;

    std::string createTmpFilename()
    {
      // TODO: This should be mutexed

      // Create some random data
      char buf[10];
      for(int i=0; i<10; i++)
        buf[i] = random.genBelow(256);

      // Hash it, then use 10 bytes of the hash
      return tmpDir + "/" + Spread::Hash(buf,10).toString().substr(5,10);
    }

    std::string createTmpFilename(const Spread::Hash &h)
    {
      return createTmpFilename() + "___" + h.toString();
    }

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
