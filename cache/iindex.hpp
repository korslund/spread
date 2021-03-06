#ifndef __SPREAD_ICACHEINDEX_HPP_
#define __SPREAD_ICACHEINDEX_HPP_

#include <hash/hash.hpp>
#include <stdint.h>
#include <vector>
#include <set>

/* Abstract interface and definitions for CacheIndex. For
   documentation of the class and member functions, see index.hpp.
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

  struct CIEntry
  {
    Spread::Hash hash;
    std::string file;
    int64_t writeTime;
  };

  typedef std::vector<CIEntry> CIVector;
  typedef std::set<std::string> StrSet;

  struct ICacheIndex
  {
    virtual int getStatus(const std::string &where, const Spread::Hash &hash) = 0;
    virtual std::string findHash(const Spread::Hash &hash) = 0;
    virtual Spread::Hash addFile(std::string where, const Spread::Hash &h = Spread::Hash(),
                                 bool allowMissing=false) = 0;
    virtual void addMany(const Spread::Hash::DirMap &files,
                         const StrSet &remove = StrSet()) = 0;
    virtual void checkMany(Spread::Hash::DirMap &files) = 0;
    virtual void removeFile(const std::string &where) = 0;
    virtual void getEntries(CIVector &result) const = 0;

    // Convenience version of addFile() for when you expect the file to be missing
    Spread::Hash checkFile(const std::string &where, const Spread::Hash &h = Spread::Hash())
    { return addFile(where, h, true); }
  };
}
#endif
