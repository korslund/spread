#ifndef __SPREAD_IHASHFINDER_HPP_
#define __SPREAD_IHASHFINDER_HPP_

#include <hash/hash.hpp>
#include <vector>

namespace Spread
{
  enum HashSourceType
    {
      TST_None,         // File not found
      TST_InPlace,      // The file already exists at the target destination
      TST_File,         // Existing file in the filesystem
      TST_Download,     // File can be found at the given URL
      TST_Archive       // File can be found in an archive
    };

  struct HashSource
  {
    Hash hash;                  // The object this struct describes
    int type;                   // Where you can find it
    std::string value;          // URL or file location
    std::vector<Hash> deps;     // Dependencies
    const Hash::DirMap *dir;    // Archive directory
  };

  struct IHashFinder
  {
    /* Find a hash object and return the details in 'out'. An optional
       target location can be given to optimize the result in case the
       file is already correctly installed.

       Returns true if the file was found, false otherwise.
     */
    virtual bool findHash(const Hash &hash, HashSource &out,
                          const std::string &target="") = 0;

    /* Report a broken URL for a given hash. The URL will no longer be
       suggested by findHash(). You may try calling findHash() again
       to obtain a replacement URL.
     */
    virtual void brokenURL(const Hash &hash, const std::string &url) = 0;
  };
}

#endif
