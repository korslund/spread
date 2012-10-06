#include "unpackhash.hpp"
#include <iostream>
#include "print_dir.hpp"

using namespace std;
using namespace Spread;

typedef Hash::DirMap HMap;

void print(const HMap &mp)
{
  cout << "Index:\n";
  HMap::const_iterator it;
  for(it = mp.begin(); it != mp.end(); it++)
    {
      cout << "  " << it->second << "   - " << it->first << endl;
    }
}

string D1 = "hey\n";
string D2 = "yo man\n";
Hash H1(D1.c_str(), D1.size());
Hash H2(D2.c_str(), D2.size());

int main()
{
  UnpackHash unp;

  UnpackHash::makeIndex("test1.zip", unp.index, "_unp_test2");
  print(unp.index);

  unp.addInput(Hash(), "test1.zip");
  unp.addOutput(H2, "_unp_test1/file2.txt");
  unp.addOutput(H2, "_unp_test1/file2_copy.txt");
  unp.addOutput(H1, "_unp_test1/dir/to/file1.txt");
  unp.addOutput(H2, "_unp_test1/file2_again.txt");

  unp.run();

  printDir("_unp_test1");
  printDir("_unp_test2");

  return 0;
}
