#include "listwriter.hpp"
#include <iostream>
#include "cache/index.hpp"

#include "print_dir.hpp"

using namespace SpreadGen;
using namespace Spread;
using namespace std;

Cache::CacheIndex cache;
RuleSet rules;
PackLister lst(cache, rules);

Hash dirHash, dirHash2;
Hash hello("hello",5);
Hash world("world",5);

Hash arcHash("ARC_WITH_DIR");
Hash arcHash2("OTHER_ARC");

int main()
{
  Hash::DirMap dir;
  dir["world.txt"] = world;
  dirHash2 = Dir::write(dir, "_writer_dir2.out");

  dir["hello.txt"] = hello;
  dirHash = Dir::write(dir, "_writer_dir.out");

  cache.addFile("_writer_dir.out");
  cache.addFile("_writer_dir2.out");
  rules.addURL(hello, "url-to-hello");
  rules.addURL(hello, "other-hello-url", 3, 2);
  rules.addURL(Hash("blah"), "you will never see this");
  rules.addArchive(arcHash2, dirHash2);
  rules.addArchive(arcHash, dirHash);
  rules.addURL(arcHash2, "url-to-arc2", 12, 3.4);
  rules.addURL(arcHash, "url-to-arc");

  lst.addDir("no-hint", dirHash);
  lst.setVersion("no-hint", "1.2.3");
  //lst.addHint("with-hint", arcHash2);
  lst.addDir("with-hint", dirHash);

  ListWriter wrt(cache);

  wrt.write(lst, "_writer/");

  printDir("_writer");

  return 0;
}
