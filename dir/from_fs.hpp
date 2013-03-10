#ifndef __SPREAD_DIR_FROM_FS_HPP_
#define __SPREAD_DIR_FROM_FS_HPP_

#include "cache/iindex.hpp"

namespace Spread
{
  namespace Dir
  {
    /* Load file names and hashes from the filesystem into a directory.

       Parameters:

       - dir: the output directory structure. New entries are just
         added on top of existing entries, if any.

       - path: the directory to traverse. All file paths are added
         relative to this path per default (also see: prefix)

       - cache: hash index to use. cache.addFile() is used to obtain
         all file hashes

       - recurse: recurse into subdirectories

       - includeDirs: include directory names. These hash as null
         hashes ("00") and names are terminated with "/"

       - add prefix to all names
     */
    extern void fromFS(Hash::DirMap &dir, const std::string &path,
                       Cache::ICacheIndex &cache,
                       bool recurse = true, bool includeDirs = false,
                       const std::string &prefix = "");
  };
}
#endif
