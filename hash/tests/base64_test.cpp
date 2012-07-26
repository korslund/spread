#include "hash.hpp"

#include <iostream>
using namespace std;

using namespace Spread;

void test(const Hash &h)
{
  Hash h2(h.toString());
  assert(h2 == h);
  cout << "Matched " << h << endl;
}

int main()
{
  Hash h("IC1yZiBDTWFrZ-ZpbGVzLyBDTWFrZU_hY2hlLnR4dCBjbWFrZV9pb");
  cout << "Hash = " << h << endl;
  assert(h == Hash("IC1yZiBDTWFrZ+ZpbGVzLyBDTWFrZU/hY2hlLnR4dCBjbWFrZV9pb"));
  assert(h == Hash("IC1yZiBDTWFrZ-ZpbGVzLyBDTWFrZU/hY2hlLnR4dCBjbWFrZV9pb=="));
  assert(h == Hash("IC1yZiBDTWFrZ-ZpbGVzLyBDTWFrZU/hY2hlLnR4dCBjbWFrZV9pb="));
  assert(h == Hash("IC1yZiBDTWFrZ-ZpbGVzLyBDTWFrZU_hY2hlLnR4dCBjbWFrZV9pbAA"));
  assert(h == Hash("IC1yZiBDTWFrZ-ZpbGVzLyBDTWFrZU_hY2hlLnR4dCBjbWFrZV9pbA="));

  test(h);
  test(Hash("hello",5));
  test(string("________________________________________"));
  test(string("________________________________________AAA===AA=A===A"));
  test(string("______________________________________________________"));
  test(string("____________________________________________________w"));
  test(string("_____________________________________________________w"));
  test(string("_____________________________________________________w=="));
  test(string("AAx"));
  test(string("==x=="));
  test(string("+-+-/_/_-=+/"));
  test(string("__//++--+/-_"));

  return 0;
}
