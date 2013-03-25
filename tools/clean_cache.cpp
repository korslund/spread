#include <iostream>
#include <cache/index.hpp>

using namespace std;
using namespace Cache;
using namespace Spread;

int main(int argc, char **argv)
{
  if(argc != 2)
    {
      cout << "Syntax: clean_cache <cache-file>\n";
      return 1;
    }

  string file = argv[1];
  cout << "Loading " << file << "\n";
  CacheIndex cache(argv[1]);
  cout << "Cleaning...\n";
  cache.verify();
  cout << "Done!\n";

  return 0;
}
