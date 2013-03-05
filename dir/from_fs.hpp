#ifndef __SPREAD_DIR_FROM_FS_HPP_
#define __SPREAD_DIR_FROM_FS_HPP_

#include "cache/iindex.hpp"

namespace Spread
{
  /*
    Structure used to load a directory from a file system dir. It has
    various optional elements you can set to control the process
    before running load().

    The struct is reusable and has no internal state (except through
    the CacheIndex.) You can call load() multiple times independently
    of each other.
   */
  struct DirFromFS
  {
    // Hash index cache to use. The cache.addFile() member function is
    // used to obtain all hashes.
    Cache::ICacheIndex &cache;

    // Set to true (default) to recurse into subdirectories
    bool recurse;

    // Set to true (NOT default) to include directory names in
    // output. These are added as Null ("00") hashes.
    bool includeDirs;

    // Set prefix to add to all added filenames (eg. "some/subdir/").
    // Default is no prefix; filenames are added relative to the base
    // path.
    std::string prefix;

    DirFromFS(Cache::ICacheIndex &_cache)
      : cache(_cache)
      , recurse(true)
      , includeDirs(false)
    {}

    void load(const std::string &path, Hash::DirMap &dir) const;
  };
}
#endif
