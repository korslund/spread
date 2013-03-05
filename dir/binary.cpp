#include "binary.hpp"
#include <mangle/stream/servers/file_stream.hpp>
#include <mangle/stream/servers/outfile_stream.hpp>
#include <mangle/stream/servers/null_stream.hpp>
#include "hash/hash_stream.hpp"
#include <vector>

using namespace Spread;
using namespace std;
using namespace Mangle::Stream;

typedef Hash::DirMap DirMap;

const int MAGIC_NUMBER = 0x4d3bface;

static void fail(const std::string &msg) { throw std::runtime_error(msg); }
static void failDir(const std::string &msg)
{ fail("Error parsing directory file: " + msg); }

Hash Spread::Dir::read(Hash::DirMap &dir, StreamPtr strm)
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

Hash Spread::Dir::write(const Hash::DirMap &dir, StreamPtr strm)
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

Hash Spread::Dir::read(Hash::DirMap &dir, const std::string &file)
{ return read(dir, FileStream::Open(file)); }

Hash Spread::Dir::write(const Hash::DirMap &dir, const std::string &file)
{ return write(dir, OutFileStream::Open(file)); }

Hash Spread::Dir::hash(const Hash::DirMap &dir)
{
  return write(dir, StreamPtr(new NullStream));
}
