#include <iostream>

#include "index.hpp"

using namespace Spread;
using namespace std;

Cache::CacheIndex index;

Hash add(const std::string &file, const Hash &h = Hash())
{
  cout << "Adding " << file;
  if(!h.isNull()) cout << " (expecting " << h << ")";
  cout << endl;
  Hash out;
  try
    {
      out = index.addFile(file,h);
      cout << "  Got: " << out << endl;
    }
  catch(exception &e)
    {
      cout << "  Error: " << e.what() << endl;
    }
  return out;
}

void find(const Hash &h)
{
  cout << "Searching for " << h << endl;
  cout << "  Found: " << index.findHash(h) << endl;
}

void status(const std::string &file, const Hash &h)
{
  using namespace Cache;

  cout << "Checking status for " << file << "   Hash: " << h << "\n  ";
  switch(index.getStatus(file,h))
    {
    case CI_None: cout << "No file found\n"; break;
    case CI_Match: cout << "File already matches\n"; break;
    case CI_Diff: cout << "File exists but is different:\n    "
                       << index.addFile(file) << endl; break;
    case CI_ElseWhere: cout << "Hash exists elsewhere: " << index.findHash(h) << endl;
      break;
    default:
      assert(0);
    }
}

int main()
{
  index.load("cache2.conf");

  Hash hello("hello", 5);
  status("hello2.dat", hello);
  status("hello.dat", hello);

  return 0;
}
