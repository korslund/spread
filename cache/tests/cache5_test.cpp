#include <iostream>
#include <boost/filesystem.hpp>

#include "index.hpp"

using namespace Spread;
using namespace std;
using namespace boost::filesystem;

Cache::CacheIndex cache;

Hash test("TEST");

int main()
{
  /* Regression test.

     This was similar to cache4_test, but with an existing,
     non-matching file.
   */
  remove("_cache5.conf");
  copy_file("cache5.conf", "_cache5.conf");
  cache.load("_cache5.conf");

  // Fixed infinite loop
  cout << "Start\n";
  cout << "FILE: " << cache.findHash(test) << endl;
  cout << "End\n";

  return 0;
}
