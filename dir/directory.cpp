#include "directory.hpp"
#include "misc/readjson.hpp"
#include <mangle/stream/servers/file_stream.hpp>
#include <mangle/stream/servers/outfile_stream.hpp>
#include <mangle/stream/servers/null_stream.hpp>
#include "hash/hash_stream.hpp"
#include <vector>

using namespace Spread;
using namespace std;
using namespace Json;
using namespace Mangle::Stream;

typedef Hash::DirMap DirMap;

void Directory::parseJson(const Value &root)
{
  Value::Members keys = root.getMemberNames();
  Value::Members::iterator it;
  for(it = keys.begin(); it != keys.end(); it++)
    {
      const string &key = *it;
      const string &val = root[key].asString();

      dir[key] = Hash(val);
    }
}

Value Directory::makeJson() const
{
  Value root;

  DirMap::const_iterator it;
  for(it = dir.begin(); it != dir.end(); it++)
    root[it->first] = it->second.toString();

  return root;
}

void Directory::readJson(const string &file)
{
  parseJson(ReadJson::readJson(file));
}

void Directory::writeJson(const string &file) const
{
  ReadJson::writeJson(file, makeJson());
}

Hash empty_hash;

const Hash &Directory::find(const std::string &name) const
{
  DirMap::const_iterator it = dir.find(name);
  if(it == dir.end())
    return empty_hash;

  return it->second;
}

const int MAGIC_NUMBER = 0x4d3bface;

static void fail(const std::string &msg) { throw std::runtime_error(msg); }
static void failDir(const std::string &msg)
{ fail("Error parsing directory file: " + msg); }

Hash Directory::read(StreamPtr strm)
{
  HashStream inf(strm);

  int tmp;

  // Read magic number
  inf.read(&tmp, 4);
  if(tmp != MAGIC_NUMBER)
    failDir("invalid magic number");

  // Number of hashes
  inf.read(&tmp, 4);

  // If someone throws us a malicious object, better to throw than
  // to crash.
  if(tmp > 1024*1024)
    failDir("too many directory hashes");

  Hash h;
  uint8_t hbuf[40];
  vector<char> buf;
  for(int i=0; i<tmp; i++)
    {
      // Read and parse hash
      inf.read(hbuf, 40);
      h.copy(hbuf);

      // String length + data
      int len = 0;
      inf.read(&len, 2);
      if(buf.size() < len)
        buf.resize(len);
      inf.read(&buf[0], len);

      // Store
      dir[string(&buf[0], len)] = h;
    }

  return inf.finish();
}

Hash Directory::write(StreamPtr strm) const
{
  HashStream ouf(strm);

  int tmp;

  tmp = MAGIC_NUMBER;
  ouf.write(&tmp, 4);

  tmp = dir.size();
  ouf.write(&tmp, 4);

  for(DirMap::const_iterator it = dir.begin();
      it != dir.end(); it++)
    {
      ouf.write(it->second.getData(), 40);

      const void *p = it->first.c_str();
      tmp = it->first.size();

      // Write size (2 bytes) first, then the string it self
      ouf.write(&tmp, 2);
      ouf.write(p, tmp);
    }

  return ouf.finish();
}

Hash Directory::read(const std::string &file)
{ return read(FileStream::Open(file)); }

Hash Directory::write(const std::string &file) const
{ return write(OutFileStream::Open(file)); }

Hash Directory::hash() const
{
  return write(StreamPtr(new NullStream));
}

void Directory::add(const DirMap &d)
{
  for(DirMap::const_iterator it = d.begin();
      it != d.end(); it++)
    dir[it->first] = it->second;
}
