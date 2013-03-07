#include "tools.hpp"
#include <iostream>

using namespace std;
using namespace Spread;

void print(const Hash::DirMap &dir)
{
  Hash::DirMap::const_iterator it;
  for(it = dir.begin(); it != dir.end(); it++)
    {
      const std::string &file = it->first;
      const Hash &hash = it->second;
      assert(file != "");
      cout << hash << " " << file << endl;
    }
  cout << endl;
}

int main()
{
  Hash::DirMap dir, sum;

  dir["abc"] = Hash("ABC");
  dir["def"] = Hash("DEF");

  Dir::add(sum, dir);
  print(sum);

  Dir::add(sum, dir, "prefix_");
  print(sum);

  dir["abc"] = Hash("GHIIIII");
  Dir::add(sum, dir);
  print(sum);

  return 0;
}
