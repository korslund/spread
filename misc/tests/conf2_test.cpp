#include "jconfig.hpp"

#include <iostream>
#include <assert.h>

using namespace std;
using namespace Misc;

int main()
{
  map<string,string> mp;

  mp["a"] = "a";
  mp["b"] = "b";
  mp["c"] = "c";
  mp["d"] = "d";
  mp["e"] = "e";
  mp["f"] = "f";

  JConfig conf1;
  conf1.setMany(mp);
  conf1.save("_conf2_1.txt");
  JConfig conf2("_conf2_2.txt");
  conf2.setMany(mp);

  set<string> st;
  st.insert("a");
  st.insert("e");

  JConfig conf3;
  conf3.setMany(mp, st);
  conf3.save("_conf2_3.txt");
  return 0;
}
