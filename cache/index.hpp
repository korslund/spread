#ifndef __SPREAD_CACHEINDEX_HPP_
#define __SPREAD_CACHEINDEX_HPP_

#include "iindex.hpp"

/* The cache index holds a list over files and their hashes. It
   verifies and updates its own information on the fly as member
   functions are called.

   The index doesn't actually store, write or modify any files (except
   for its own config file), it just indexes existing files.

   The class is thread safe.
 */

namespace Cache
{
  struct CacheIndex : ICacheIndex
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

       Throws if the file does not exist, unless if allowMissing=true,
       in which case a missing file causes a null hash return value.

       Postcondition: the information about the file in our cache
       matches reality. Exceptions thrown will not put the object in
       an invalid state.

       If a hash is given, we trust that it is an up-to-date and
       correct hash of the current file data. We will double-check the
       size and throw an exception if it doesn't match the given hash
       size.

       Return value: the hash of the file, either from cache or from
       rehashing the file.
     */
    Spread::Hash addFile(std::string where, const Spread::Hash &h = Spread::Hash(),
                         bool allowMissing=false);

    /* Used to mass add a bunch of files to the index. The function
       works more or less like addFile(), but is called on a list of
       files in a DirMap, with their associated hashes. Entries with
       null-hashes are read from disk and hashed. Entries with
       non-null hashes must match the data in the file system.

       For large groups of files, this is potentially much faster than
       running addFile() on each individual file. This is because
       addFile() will normally write out the cache config file after
       each addition, which can be very time consuming when the index
       is large.
     */
    void addMany(const Spread::Hash::DirMap &files);

    /* Remove a file entry from the cache. Doesn't actually delete the
       file.

       Non-existing entries are ignored.
     */
    void removeFile(const std::string &where);

    /* Load data from a file. This file will be kept continually
       updated with changes from this point on.

       It's OK to specify a file that doesn't exist; it will be
       created on demand.
     */
    void load(const std::string &conf);

    /* Get a complete list of all the entries in the index.
     */
    void getEntries(CIVector &result) const;

    // The optional 'conf' parameter is passed on to load().
    CacheIndex(const std::string &conf = "");
    ~CacheIndex();
  private:
    struct _CacheIndex_Hidden;
    _CacheIndex_Hidden *ptr;
    Spread::Hash addEntry(std::string &where, const Spread::Hash &given,
                          uint64_t &time);

  };
}
#endif
