#ifndef __SPREAD_BINARY_DIR_HPP_
#define __SPREAD_BINARY_DIR_HPP_

#include <hash/hash.hpp>
#include <mangle/stream/stream.hpp>

namespace Spread
{
  namespace Dir
  {
    /* Read and write binary dir files. Returns the hash of the data
       read or written.
     */
    extern Hash read(Hash::DirMap &dir, const std::string &file);
    extern Hash write(const Hash::DirMap &dir, const std::string &file);

    // Stream versions
    extern Hash read(Hash::DirMap &dir, Mangle::Stream::StreamPtr strm);
    extern Hash write(const Hash::DirMap &dir, Mangle::Stream::StreamPtr strm);

    /* Return the hash of the dir, without actually writing the data
       anywhere.
     */
    extern Hash hash(const Hash::DirMap &dir);
  };
}
#endif
