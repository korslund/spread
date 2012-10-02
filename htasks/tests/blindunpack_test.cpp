#include "unpackhash.hpp"
#include <iostream>
#include "print_dir.hpp"

using namespace std;
using namespace Spread;

typedef UnpackHash::HashMap HMap;

void print(const HMap &mp)
{
  cout << "Index:\n";
  HMap::const_iterator it;
  for(it = mp.begin(); it != mp.end(); it++)
    {
      cout << "  " << it->second << "   - " << it->first << endl;
    }
}

int main()
{
  HMap index;
  UnpackHash unp("_blind_unp", index, true);
  unp.addInput(Hash(), "test1.zip");
  cout << "Staring unpack job\n";
  unp.run();
  print(index);
  printDir("_blind_unp");
  return 0;
}
