#include "hash.hpp"

#include <assert.h>
#include <iostream>
using namespace std;
using namespace Spread;

int main()
{
  {
    Hash h;
    h.update("aaa", 3);
    Hash a = h;
    // Test that destructors don't double-delete
  }
  {
    Hash h;
    h.update("aaa", 3);
    Hash a = h;
    cout << h.finish() << endl;
    assert(h == a.finish());
    // Test that finish on both ends work
  }

  // Repeat with assignment rather than initialization
  {
    Hash h;
    h.update("aaa", 3);
    Hash a;
    a = h;
  }
  {
    Hash h;
    h.update("aaa", 3);
    Hash a;
    a = h;
    cout << h.finish() << endl;
    assert(h == a.finish());
  }

  cout << "aaaa: " << Hash("aaaa", 4) << endl;
  cout << "aaab: " << Hash("aaab", 4) << endl;
  cout << "aaac: " << Hash("aaac", 4) << endl;

  {
    Hash a;
    a.update("aaa", 3);
    Hash b = a, c; c=a;
    b.update("b", 1);
    b.finish();
    a.update("a", 1);
    c.update("c", 1);
    cout << a.finish() << endl << b << endl << c.finish() << endl;
  }

  return 0;
}
