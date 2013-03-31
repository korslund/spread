#include "packlist.hpp"
#include <iostream>

using namespace std;
using namespace Spread;

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

void print(const PackList &list)
{ print(list.getList()); }

int main()
{
  PackList list;
  list.loadJson("data_packs1.json", "test");
  print(list.get("shots300x260"));
  try { list.get("error"); }
  catch(exception &e) { cout << "Expected exception: " << e.what() << endl; }
  print(list);
  list.loadJson("data_packs2.json", "test");
  print(list);
  list.clear();
  print(list);

  return 0;
}
