#ifndef __SPREAD_CACHE_FILES_HPP_
#define __SPREAD_CACHE_FILES_HPP_

#include "iindex.hpp"

namespace Cache
{
  struct Files
  {
    Files(ICacheIndex &_index, const std::string &_dir = "")
      : basedir(_dir), index(_index) {}

    /* Set this (directly or through the constructor) before using the
       object.
     */
    std::string basedir;

    /* Produce cache path for a given hash. This does not read or
       write the filesystem, it just creates the path string.
     */
    std::string makePath(const Spread::Hash &hash) const;

    /* Get a path ready for storing files. This will check if the file
       already exists, with the right hash. If it does, the function
       returns "". If a string is returned, the file is guaranteed not
       to exist.

       All necessary parent directories will be created.
     */
    std::string storePath(const Spread::Hash &hash) const;

  private:
    ICacheIndex &index;
  };
}

#endif
