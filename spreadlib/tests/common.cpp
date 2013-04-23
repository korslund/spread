#include "../../spread.hpp"
#include <iostream>
#include <boost/filesystem.hpp>
#include "print_dir.hpp"

using namespace Spread;
using namespace std;
namespace bf = boost::filesystem;

void print(const PackInfo &pack)
{
  cout << "PACK: " << pack.package
       << "\nChannel: " << pack.channel
       << "\nVersion: " << pack.version
       << "\nSize: " << pack.installSize
       << "\nDirs:\n";
  assert(pack.dirs.size() == pack.paths.size());
  for(int i=0; i<pack.dirs.size(); i++)
    cout << "  " << pack.dirs[i] << "   " << pack.paths[i] << endl;
  cout << endl;
}

void print(const PackInfoList &list)
{
  cout << "ALL PACKS:\n";
  for(int i=0; i<list.size(); i++)
    print(list[i]);
}

void print(const PackStatus &pack)
{
  cout << "PACK: " << pack.info.package
       << "\n  Channel: " << pack.info.channel
       << "\n  Version: " << pack.info.version
       << "\n  Size: " << pack.info.installSize
       << "\n  Dirs:\n";
  assert(pack.info.dirs.size() == pack.info.paths.size());
  for(int i=0; i<pack.info.dirs.size(); i++)
    cout << "    " << pack.info.dirs[i] << "   " << pack.info.paths[i] << endl;
  cout << "  STATUS: where=" << pack.where << " needsUpdate=" << pack.needsUpdate << "\n";
}

void print(const PackStatus *pack)
{
  if(pack) print(*pack);
  else cout << "NULL!\n";
}

void print(const PackStatusList &list)
{
  if(list.size() == 0) cout << "(empty status list)\n";
  PackStatusList::const_iterator it;
  for(it = list.begin(); it != list.end(); it++)
    print(*it);
  cout << endl;
}

void printStatus(const SpreadLib &list, const std::string &channel="", const std::string &package="", const std::string &where="")
{
  PackStatusList out;
  list.getStatusList(out, channel, package, where);
  print(out);
}

void shortStatus(const SpreadLib &list)
{
  PackStatusList out;
  list.getStatusList(out);
  if(out.size() == 0) cout << "(empty status list)\n";
  PackStatusList::const_iterator it;
  for(it = out.begin(); it != out.end(); it++)
    {
      const PackStatus &p = **it;
      cout << p.info.channel << "/" << p.info.package << " d=" << p.info.dirs[0]
           << " NEED=" << p.needsUpdate << "\n";
    }
  cout << endl;
}
