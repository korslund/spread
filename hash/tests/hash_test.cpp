#include "hash.hpp"

#include <assert.h>
#include <iostream>
using namespace std;

using namespace Spread;

int main()
{
  Hash h;

  assert(h.isNull());
  cout << "Initial value:\n" << h << endl;
  h.size() = 10;
  assert(!h.isNull());
  cout << "With size = 10:\n" << h << endl;
  h.size() = 123456789;
  cout << "With size = 123456789:\n" << h << endl;
  cout << "Hash string size: " << h.toString().size() << endl;

  cout << "Hash of 'Hello Dolly':\n" << Hash("Hello Dolly", 11) << endl;
  cout << "'This is Louis, Dolly\\n\n" << Hash("This is Louis, Dolly\n", 21) << endl;

  if(Hash("abc", 3) == Hash("abc", 3)) cout << "Got expected match\n";
  if(Hash("abc", 3) != Hash("abd", 3)) cout << "Got expected mismatch\n";
  assert(!(Hash("abc", 3) == Hash("abd", 3)));
  assert(!(Hash("abc", 3) != Hash("abc", 3)));

  cout << "From string:\n" << Hash("0123456789abcdef0123456789abcdef00112233445566778899aabbccddeeff8888888811111111") << endl;

  h.hash("abcd", 4);
  cout << "abcd:\n" << h << endl;
  h.fromString(h.toString());
  cout << "to and from string:\n" << h << endl;

  cout << "\nHash of zero buffer:\n" << Hash(NULL,0) << endl;
  cout << "Finishing early:\n" << h.finish() << endl << h.finish() << endl;

  h.update("ab", 2);
  h.update("cd", 2);
  cout << "Hashed abcd in two turns:\n" << h.finish() << endl;
  h.update("a", 1);
  h.update("bc", 2);
  h.update("d", 1);
  h.finish();
  cout << "Repeat with 3 turns:\n" << h << endl;

  return 0;
}
