#include "sr0_gen.hpp"
#include <iostream>

using namespace SpreadGen;
using namespace Spread;
using namespace std;

Cache::Cache cache;
RuleSet rules;

Hash dirHash;
Hash hello("hello",5);
Hash world("world",5);

Hash arcHash("ARC_WITH_DIR");

int main()
{
  Directory dir;
  dir.dir["file1.txt"] = hello;
  dir.dir["file2.txt"] = world;
  dirHash = dir.write("_sr0_dir.out");
  cache.index.addFile("_sr0_dir.out");

  rules.addURL(hello, "url-to-hello");
  rules.addURL(arcHash, "url-to-archive");
  rules.addArchive(arcHash, dirHash);

  GenSR0 gen(cache, rules);
  gen.makeSR0(dirHash, "_sr0_output");

  return 0;
}
