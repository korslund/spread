#ifndef __SPREAD_GEN_LISTWRITER_HPP_
#define __SPREAD_GEN_LISTWRITER_HPP_

#include "packlister.hpp"
#include <mangle/vfs/stream_factory.hpp>
//#include "misc/jconfig.hpp"

/* The ListWriter takes the output from a PackLister and writes it as
   files to a directory.

   The current SR0-compatible format has the following files:

   - packs.json       index of packages
   - rules.json       rule file
   - 1kcache.json     list of 1k hashes (not implemented yet)
   - dirs/*           directory object files

   The format is optimized for readability and ease of use, not size.
   Future versions may be much more size-optimized. Specifically:

   - write all data in binary (optimized for small bsdiffs)
   - hashes written in one place, sorted and size-compressed in binary
     (sorting tends to increase compressibility)
   - all the rules, packs and dirs reference a shared hash list, but
     in a way so that references don't change needlessly between
     updates.
 */

namespace SpreadGen
{
  struct ListWriter
  {
    ListWriter(Cache::Cache &_cache/*, const Misc::JConfig &_c1k*/)
      : cache(_cache)/*, cache1k(_c1k)*/ {}

    // Write to a directory
    void write(const PackLister &lister, const std::string &where);

    // Write to a stream factory (or "virtual directory")
    void write(const PackLister &lister, Mangle::VFS::StreamFactoryPtr output);

  private:
    Cache::Cache &cache;
    //Misc::JConfig &cache1k;
  };
}

#endif
