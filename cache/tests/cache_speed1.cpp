#include <iostream>

#include "timer.hpp"
#include "index.hpp"
#include <boost/filesystem.hpp>
#include <mangle/stream/servers/outfile_stream.hpp>

using namespace Spread;
using namespace std;
using namespace boost::filesystem;
using namespace Mangle::Stream;

Hash hello("hello", 5);

#define SIZE 10000

int main()
{
  remove_all("_speed1");
  create_directories("_speed1");
  OutFileStream::Write("_speed1/0", "hello");

  {
    cout << "Adding one file " << SIZE << " times (no cache file)\n";
    Timer t;
    Cache::CacheIndex index;
    for(int i=0; i<SIZE; i++)
      index.addFile("_speed1/0", hello);
    cout << "Elapsed time: " << t.total() << " secs\n";
  }

  {
    cout << "Adding one file " << SIZE << " times (WITH cache file)\n";
    Timer t;
    Cache::CacheIndex index("_speed1/cache1.conf");
    for(int i=0; i<SIZE; i++)
      index.addFile("_speed1/0", hello);
    cout << "Elapsed time: " << t.total() << " secs\n";
  }

  cout << "Writing test data\n";
  char buf[10];
  vector<string> files;
  for(int i=0; i<SIZE; i++)
    {
      snprintf(buf,10,"%d",i);
      string file = "_speed1/" + string(buf);
      files.push_back(file);
      OutFileStream::Write(file, "hello");
    }

  {
    cout << "Adding " << SIZE << " files (no cache file)\n";
    Timer t;
    Cache::CacheIndex index;
    for(int i=0; i<SIZE; i++)
      index.addFile(files[i], hello);
    cout << "Elapsed time: " << t.total() << " secs\n";
  }

  if(SIZE > 2000)
    cout << "SKIPPING slow test for SIZE > 2000\n";
  else
    {
      cout << "Adding " << SIZE << " files (WITH cache file)\n";
      Timer t;
      Cache::CacheIndex index("_speed1/cache2.conf");
      for(int i=0; i<SIZE; i++)
        index.addFile(files[i], hello);
      cout << "Elapsed time: " << t.total() << " secs\n";
    }

  {
    cout << "Mass adding " << SIZE << " files (WITH cache file)\n";
    Timer t;
    Cache::CacheIndex index("_speed1/cache3.conf");
    Hash::DirMap hashes;
    for(int i=0; i<SIZE; i++)
      hashes[files[i]] = hello;
    index.addMany(hashes);
    cout << "Elapsed time: " << t.total() << " secs\n";
  }

  return 0;
}
