#include <iostream>
#include <boost/filesystem.hpp>

#include "index.hpp"

using namespace Spread;
using namespace std;
using namespace boost::filesystem;

Cache::CacheIndex cache;

Hash hello("hello",5);

int main()
{
  // Regression test

  copy_file("hello.dat", "_t3_out.dat");

  cache.addFile("_t3_out.dat");
  cache.addFile("hello.dat");

  cout << cache.findHash(hello) << endl;

  remove("_t3_out.dat");

  // FIXED segfault here
  cout << cache.findHash(hello) << endl;

  return 0;
}
