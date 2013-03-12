#include <cache/index.hpp>
#include <dir/from_fs.hpp>
#include <dir/binary.hpp>
#include <iostream>

void printDir(const Spread::Hash::DirMap &dir)
{
  using namespace Spread;
  using namespace std;

  Hash::DirMap::const_iterator it;
  for(it = dir.begin(); it != dir.end(); it++)
    {
      string hstring = it->second.toString();

      // Pad hashes up to 50 chars
      for(int i=hstring.size(); i<50; i++)
        hstring += " ";

      cout << hstring << " " << it->first << endl;
    }
  cout << "Total " << dir.size() << " elements\n";;
  cout << "Hash: " << Dir::hash(dir) << endl;
}

void printDir(const std::string &where)
{
  using namespace Spread;
  using namespace std;

  Hash::DirMap dir;
  Cache::CacheIndex cache;
  Dir::fromFS(dir, where, cache, true, true);
  cout << "\nDirectory: " << where << endl;
  printDir(dir);
}
