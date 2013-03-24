#include <iostream>

#include "index.hpp"

using namespace Spread;
using namespace std;

Cache::CacheIndex index;
Hash hello("hello",5);

void dump()
{
  cout << "INDEX:\n";
  Cache::CIVector vec;
  index.getEntries(vec);
  for(int i=0; i<vec.size(); i++)
    {
      Cache::CIEntry &e = vec[i];
      cout << e.hash << " " << e.file << " " << e.writeTime << endl;
    }
  cout << endl;
}

int main()
{
  Hash::DirMap files;
  files["hello.dat"] = hello;
  files["test.sh"];
  files["nothing"];

  cout << "Checking files:\n";
  index.checkMany(files);
  cout << "  test.sh: " << files["test.sh"] << endl;
  cout << "  nothing: " << files["nothing"] << endl;
  dump();

  cout << "Updating list:\n";
  files.clear();
  files["hello.dat"];
  files["hello2.dat"];
  Cache::StrSet st;
  st.insert("test.sh");
  index.addMany(files, st);
  dump();

  return 0;
}
