#include <iostream>

#include "index.hpp"
#include "files.hpp"

#include <boost/filesystem.hpp>
#include <mangle/stream/servers/outfile_stream.hpp>

using namespace Spread;
using namespace std;
namespace bf = boost::filesystem;

Cache::CacheIndex cache;
Cache::Files files(cache, "_files");

void test(const Hash &hash)
{
  cout << "HASH: " << hash << endl;
  cout << "Path: " << files.makePath(hash) << endl;

  std::string path = files.storePath(hash);
  cout << "Storing to " << path << endl;
  Mangle::Stream::OutFileStream::Write(path, "hello");
  cout << "Storing again: " << files.storePath(hash) << endl;
  cout << "        twice: " << files.storePath(hash) << endl;
}

int main()
{
  bf::remove_all("_files");

  test(Hash("abcdef"));
  test(Hash("hello",5));

  return 0;
}
