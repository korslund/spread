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

     Input:
     - cache file contains a legacy relative (non-absolute) path to a
       file that no longer exists

     What happened:
     - while searching for the path, CacheIndex found the non-existing
       file and tried to remove the entry. However (I assume) it tried
       to remove the absolute path rather than the relative one, and
       because of the way the loop was formulated internally in
       CacheIndex, this caused an infinite loop.
   */
  remove("_cache4.conf");
  copy_file("cache4.conf", "_cache4.conf");
  cache.load("_cache4.conf");

  // Fixed infinite loop
  cout << "Start\n";
  cout << "FILE: " << cache.findHash(test) << endl;
  cout << "End\n";

  return 0;
}
