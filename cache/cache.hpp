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
  };
}

#endif
