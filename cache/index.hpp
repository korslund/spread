#ifndef __SPREAD_CACHEINDEX_HPP_
#define __SPREAD_CACHEINDEX_HPP_

#include "hash/hash.hpp"

/* The cache index holds a list over files and their hashes. It
   verifies and updates its own information on the fly as member
   functions are called.

   The index doesn't actually store, write or modify any files (except
   for its own config file), it just indexes existing files.
 */

namespace Cache
{
  // Status of a file location
  enum CIStatus
    {
      CI_None,          // The file does not exist
      CI_Match,         // File exists, and matches the given hash
      CI_Diff,          // File exists, but differs from the given
                        // hash. Use addFile() to get it.

      CI_ElseWhere      /* File does not match, but we have this file
                           somewhere else. Use findHash() to get
                           it. This takes precedence over CI_Diff if
                           both apply, since we assume a complete file
                           somewhere else is more valuable than a
                           potentially patchable file in the correct
                           place.
                        */
    };

  struct CacheIndex
  {
    /* Returns a CIStatus result for a given file location and
       hash. Will check the file and cache to make sure returned
       results are consistent with reality.

       This is useful to call when you are about to write to a new
       location, as knowledge of already existing files might save
       some work.
     */
    int getStatus(const std::string &where, const Spread::Hash &hash);

    /* Find any file matching the given hash. Returns an empty string
       if nothing was found.
     */
    std::string findHash(const Spread::Hash &hash);

    /* Add the given location to our cache, or confirm/refresh the
       entry if it already exists.

       The function uses last_write_time() and file_size() to detect
       whether a file needs to be rehashed.

       Postcondition: the information about the file in our cache
       matches reality.

       If a hash is given, we trust that it is an up-to-date and
       correct hash of the current file data. We will double-check the
       size and throw an exception if it doesn't match the given hash
       size.

       Return value: the hash of the file, either from cache or from
       rehashing the file.
     */
    Spread::Hash addFile(const std::string &where,
                         const Spread::Hash &h = Spread::Hash());

    /* Remove a file entry from the cache. Doesn't actually delete the
       file.

       Non-existing entries are ignored.
     */
    void removeFile(const std::string &where);

    CacheIndex();
    ~CacheIndex();
  private:
    struct _CacheIndex_Hidden;
    _CacheIndex_Hidden *ptr;
  };
}

#endif
