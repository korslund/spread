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
  dir.write("test.bin");

  Directory dir2;
  dir2.read("test.bin");
  cout << dir2.makeJson();

  return 0;
}
