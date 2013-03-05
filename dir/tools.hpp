#ifndef __SPREAD_DIR_TOOLS_HPP_
#define __SPREAD_DIR_TOOLS_HPP_

#include "hash/hash.hpp"

namespace Spread
{
  namespace Dir
  {
    /* Meld dir2 into dir. An optional prefix can be added to each
       filename.
     */
    extern void add(Hash::DirMap &dir, const Hash::DirMap &dir2,
                    const std::string &prefix = "");
  }
}
#endif
