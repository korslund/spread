#ifndef __SPREAD_HASH_STREAM_H_
#define __SPREAD_HASH_STREAM_H_

#include "hash.hpp"
#include <mangle/stream/stream.hpp>
#include <mangle/stream/filters/pure_filter.hpp>
#include <mangle/stream/servers/file_stream.hpp>
#include <mangle/stream/servers/outfile_stream.hpp>

/*
  Hash sum wrapping Mangle Stream.

  This filter stream computes the hash sum of all data read or written
  through it. It has one control function finish(), which is
  documented below. The hash sum is computed in the .hash member, and
  also returned by finish(). Otherwise the stream does not affect the
  data flow in any way.

  The class also hosts two static sum() functions, which hash a stream
  (or a file) in one go with no persistant information.

  -- More details: --

  This stream does NOT work well with:
  - pointer streams - only read() and write() calls are summed
  - seeking - unless you want some very special effects, seeking is
              not recommended while hashing.

  For this reason, hasPtr and isSeekable is set to false to prevent
  accidental misuse and hard-to-find bugs. If you need these to be
  true, set them manually.

  Also: The ptr and seek functions will still work even though the
  flags are false, so you can still do seeking or pointer operations
  if you know what you are doing.

  Some possible uses beyond simple stream hashing:

  - partial sum: calculate the hash sum of eg. the first 10k by calling
    read(buf, 10*1024) then finish(). Can be done repeatedly to sum a
    stream in blocks rather than as one sum.

  - concatenation: call setStream() multiple times and read each
    stream in full (or in part) to calculate the sum of several
    streams together into one hash.

  - uni- or bi-directional communication verification. For a socket or
    communication stream used for reading and/or writing data,
    finish() can be called in regular intervals (or at the end) to
    verify that both parties got the same result. It doesn't matter if
    one side's reads are the other's writes, as long as the order is
    the same. Can also be used for signing (eg. through sending a hmac
    of the hash) to verify your identity to the other party.
 */

namespace Spread
{

template <class Hash>
struct HashStreamT : Mangle::Stream::PureFilter
{
  Hash hash;

  // Total number of bytes summed (read + written). It is NEVER reset
  // to zero outside the constructor, so if you want to reset the
  // counter you have to do so manually.
  size_t summed;

  // Start without a stream. Use PureFilter::setStream() before you
  // begin stream operations.
  HashStreamT() : summed(0)
  {
    isSeekable = false;
    hasPtr = false;
  }

  // Start with a stream
  HashStreamT(Mangle::Stream::StreamPtr src) : summed(0)
  {
    setStream(src);
    isSeekable = false;
    hasPtr = false;
  }

  // File version (read or write, depending on second parameter)
  HashStreamT(const std::string &filename, bool write=false) : summed(0)
  {
    using namespace Mangle::Stream;
    Stream *p;
    if(write) p = new OutFileStream(filename);
    else p = new FileStream(filename);
    setStream(StreamPtr(p));
    isSeekable = false;
    hasPtr = false;
  }

  /* Finish and reset the hash summing sequence. The result is
     returned, as well as stored in the .hash member.

     Any read/write operations after finish() restarts the summing
     process, as per update()/finish() in the Hash struct.
   */
  Hash finish()
  {
    return hash.finish();
  }

  size_t read(void *buf, size_t count)
  {
    // Read, then hash
    size_t res = src->read(buf, count);
    // And make sure to only hash actual bytes read
    hash.update(buf, res);
    summed += res;
    return res;
  }

  size_t write(const void *buf, size_t count)
  {
    // Hash, then write
    hash.update(buf, count);
    summed += count;
    return src->write(buf,count);
  }

  // Buffer size used for the sum() functions
  static const int DEFSIZE = 16*1024;

  /* Directly sum THIS stream. See the static sum() function for more
     details.
  */
  Hash sum(int bufsize = DEFSIZE)
  { return sum(*this, bufsize); }

  /* Directly sum an entire stream. The stream is read in increments,
     to avoid loading large files into memory. You can specify the
     buffer size as the second parameter, the default is 16 Kb.

     The algorithm does not depend on size() for non-pointer streams.

     Pointer streams are hashed in one go, and do not advance the file
     pointer.

     Since the position of the file pointer after this operation
     depends on the type of stream given, it is considered undefined.
   */
  static Hash sum(Mangle::Stream::Stream &str, int bufsize = DEFSIZE)
  {
    assert(str.isReadable);

    // Is this a memory stream?
    if(str.hasPtr)
      {
        assert(str.hasSize);

        // Pointer streams makes life easy
        return Hash(str.getPtr(), str.size());
      }

    // No pointers today. Create a buffer and hash in increments.
    char *buf = new char[bufsize];

    Hash result;

    // We do not depend on the stream size, only on eof().
    while(!str.eof())
      {
        size_t num = str.read(buf, bufsize);

        // If we read less than expected, we should be at the
        // end of the stream
        assert(num == bufsize || (num < bufsize && str.eof()));

        // Hash the data
        result.update(buf, num);
      }

    // Clean up and return
    delete[] buf;
    return result.finish();
  }
  static Hash sum(Mangle::Stream::StreamPtr str, int bufsize = DEFSIZE)
  { return sum(*str, bufsize); }

  // Filename version of sum(), through FileStream.
  static Hash sum(const std::string &filename, int buf = DEFSIZE)
  {
    using namespace Mangle::Stream;
    return sum(FileStreamPtr(new FileStream(filename)), buf);
  }
};

typedef HashStreamT<Hash> HashStream;
typedef boost::shared_ptr<HashStream> HashStreamPtr;
}
#endif
