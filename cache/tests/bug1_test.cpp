#include <iostream>
#include <mangle/stream/servers/outfile_stream.hpp>
#include <boost/filesystem.hpp>
#include <ctime>
#include "hash/hash_stream.hpp"

/* Regression test: writing the same file with the same size (but
   different content) multiple times in a row. This causes the
   date-check in addFile() to fail (false positive match), since the
   filesystem date granularity is one second, but time between file
   writes is less than one second, thus the last write is not
   detected.
 */

#include "index.hpp"

using namespace Spread;
using namespace std;

std::string file = "_bug1.dat";

int main()
{
  boost::filesystem::remove_all(file);

  Cache::CacheIndex index;

  Hash world("world",5);

  Mangle::Stream::OutFileStream::Write(file, "hello");
  Hash h1 = index.addFile(file);
  Mangle::Stream::OutFileStream::Write(file, "world");
  Hash h2 = index.addFile(file);
  Hash h3 = index.addFile(file, world);

  cout << "H1=" << h1 << "\nH2=" << h2 << "\nH3=" << h3 << endl;
  cout << "R2=" << HashStream::sum(file) << endl;
  /*
  cout << "Time: " << boost::filesystem::last_write_time(file) << endl;
  cout << "Now:  " << std::time(NULL) << endl;
  */
  return 0;
}
