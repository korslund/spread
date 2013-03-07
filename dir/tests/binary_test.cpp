#include "binary.hpp"
#include <iostream>

using namespace std;
using namespace Spread;

int main()
{
  Hash::DirMap dir;

  Hash h = Dir::read(dir, "test.bin");
  cout << "Read hash: " << h << endl;
  cout << "Elements: " << dir.size() << endl;
  cout << "Hash: " << Dir::hash(dir) << endl;
  cout << "name1: " << dir["name1"] << endl;
  cout << "dir/name2: " << dir["dir/name2"] << endl;

  return 0;
}
