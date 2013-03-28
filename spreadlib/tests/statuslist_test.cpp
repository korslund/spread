#include "statuslist.hpp"
#include <iostream>
#include <boost/filesystem.hpp>

using namespace std;
using namespace Spread;
namespace bf = boost::filesystem;

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
  cout << "ALL ELEMENTS:\n";
  PackStatusList::const_iterator it;
  for(it = list.begin(); it != list.end(); it++)
    print(*it);
  cout << endl;
}

void print(const StatusList &list)
{
  PackStatusList out;
  list.getList(out);
  print(out);
}

int main()
{
  PackInfo info;
  {
    StatusList list("");

    cout << "Listing and inserting elements:\n";
    print(list);
    info.channel = "A";
    info.package = "1";
    info.version = "Ver1";
    info.dirs.push_back("abcd");
    info.paths.push_back("");
    info.installSize = 1234;
    list.setEntry(info, "dir1/");
    print(list);
    info.package = "2";
    list.setEntry(info, "dir2/");
    list.setEntry(info, "dir3/");
    print(list);

    cout << "Individual elements:\n";
    print(list.get("A", "1"));
    print(list.get("A", "1", "dir1"));
    print(list.get("A", "1", "dir2"));
    print(list.get("A", "2"));

    cout << "Overwriting elements:\n";
    info.package = "1";
    info.version = "Ver2";
    list.setEntry(info, "dir1/");
    print(list);
    info.package = "2";
    list.setEntry(info, "dir3/");
    print(list);
    list.setEntry(info, "dir4/");
    print(list);

    cout << "Removing elements:\n";
    list.remove("A", "2", "dir3");
    print(list);
    list.remove("A", "2");
    print(list);
    list.remove("A", "1");
    print(list);
    list.remove("A", "2", "nowhere/");
    print(list);

    cout << "Notifying of new entries:\n";
    info.package = "1";
    info.version = "Ver2";
    list.setEntry(info, "dir1/");
    list.setEntry(info, "dir2/");
    print(list);
    info.installSize = 4321;
    info.version = "Ver3";
    list.notifyNew(info); // No effect
    print(list);
    info.package = "3";
    info.paths[0] = "hello/";
    list.notifyNew(info); // No effect
    print(list);
    info.package = "2";
    list.notifyNew(info); // Updates one package
    print(list);
    info.package = "1";
    list.notifyNew(info); // Updates two packages
    print(list);
    list.setEntry(info, "dir3"); // Adds one package
    print(list);
    list.setEntry(info, "dir2"); // Changes one package
    print(list);
  }

  std::string file = "_status.conf";

  bf::remove_all(file);
  bf::remove_all(file+".old");
  {
    cout << "CONFIG FILE:\n";
    StatusList list(file);
    print(list);
    list.setEntry(info, "dir1/");
    list.setEntry(info, "dir2/");
    info.package = "XXX";
    info.paths[0] = "";
    list.setEntry(info, "dir3");
    print(list);
  }
  {
    cout << "RELOAD CONFIG FILE:\n";
    StatusList list(file);
    print(list);
  }

  return 0;
}
