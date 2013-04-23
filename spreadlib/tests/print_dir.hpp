#include <cache/index.hpp>
#include <dir/from_fs.hpp>
#include <dir/binary.hpp>
#include <iostream>

void printDir(Spread::Hash::DirMap &dir, bool skipCache=false)
{
  using namespace Spread;
  using namespace std;

  Hash::DirMap::iterator it, it2;
  for(it2 = dir.begin(); it2 != dir.end();)
    {
      it = it2++;

      if(skipCache && it->first.find("cache.conf") != string::npos)
        {
          dir.erase(it);
          continue;
        }

      string hstring = it->second.toString();

      // Pad hashes up to 50 chars
      for(int i=hstring.size(); i<50; i++)
        hstring += " ";

      cout << hstring << " " << it->first << endl;
    }
  cout << "Total " << dir.size() << " elements\n";;
  cout << "Hash: " << Dir::hash(dir) << endl;
}

void printDir(const std::string &where, bool skipCache=false)
{
  using namespace Spread;
  using namespace std;

  Hash::DirMap dir;
  Cache::CacheIndex cache;
  Dir::fromFS(dir, where, cache, true, true);
  cout << "\nDirectory: " << where << endl;
  printDir(dir, skipCache);
}
