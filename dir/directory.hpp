#ifndef __SPREAD_DIRECTORY_HPP_
#define __SPREAD_DIRECTORY_HPP_

#include <map>
#include <string>
#include <json/json.h>
#include "hash/hash.hpp"
#include <mangle/stream/stream.hpp>
#include <boost/shared_ptr.hpp>

namespace Spread
{
  struct Directory
  {
    Hash::DirMap dir;

    // Read and write JSON data
    void readJson(const std::string &file);
    void parseJson(const Json::Value &val);
    void writeJson(const std::string &file) const;
    Json::Value makeJson() const;

    /* Read and write binary format. Returns the hash of the data read
       or written.
     */
    Hash read(const std::string &file);
    Hash read(Mangle::Stream::StreamPtr strm);
    Hash write(const std::string &file) const;
    Hash write(Mangle::Stream::StreamPtr strm) const;

    /* Return the hash of the dir, without actually writing the data
       anywhere.
     */
    Hash hash() const;

    /* "Meld" the given directory into this one. If you are adding
       multiple directories, the order is important. Also note that
       file path overwriting is case sensitive.
     */
    void add(const Directory &d) { add(d.dir); }
    void add(const Hash::DirMap &d);

    // Find a member entry. Returns a null hash if not found.
    const Hash &find(const std::string &name) const;
  };

  typedef boost::shared_ptr<Directory> DirectoryPtr;
  typedef boost::shared_ptr<const Directory> DirectoryCPtr;
}
#endif
