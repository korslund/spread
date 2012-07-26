#ifndef __SPREAD_DIRECTORY_HPP_
#define __SPREAD_DIRECTORY_HPP_

#include <map>
#include <string>
#include <json/json.h>
#include "hash/hash.hpp"
#include <mangle/stream/stream.hpp>

namespace Spread
{
  struct Directory
  {
    typedef std::map<std::string,Hash> DirMap;
    DirMap dir;

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

    // Find a member entry. Returns a null hash if not found.
    const Hash &find(const std::string &name) const;
  };
}

#endif
