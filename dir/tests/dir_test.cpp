#include "directory.hpp"
#include <iostream>

using namespace std;
using namespace Spread;

int main()
{
  Directory dir;

  dir.readJson("test.json");
  cout << dir.find("name1") << endl;
  cout << dir.find("name2") << endl;
  cout << dir.find("dir/name2") << endl;
  Hash h = dir.write("test.bin");
  cout << "Wrote hash: " << h << endl;
  cout << "hash() returns: " << dir.hash() << endl;

  Directory dir2;
  Hash h2 = dir2.read("test.bin");
  cout << "Read hash: " << h2 << endl;
  cout << dir2.makeJson();

  return 0;
}
