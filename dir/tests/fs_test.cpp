#include "from_fs.hpp"
#include <iostream>

using namespace std;
using namespace Spread;

Cache::CacheIndex index;
DirFromFS dfs(index);

void print(Directory &dir)
{
  Directory::DirMap::const_iterator it;
  for(it = dir.dir.begin(); it != dir.dir.end(); it++)
    {
      string hstring = it->second.toString();

      // Pad hashes up to 50 chars
      for(int i=hstring.size(); i<50; i++)
        hstring += " ";

      cout << hstring << " " << it->first << endl;
    }
  cout << "Total " << dir.dir.size() << " elements\n";;
  cout << "Hash: " << dir.hash() << endl;
}

void test(const string &msg, bool addSlash = false)
{
  cout << endl << msg << endl;
  Directory dir;
  if(addSlash) dfs.load("testdir/", dir);
  else dfs.load("testdir", dir);
  print(dir);
}

int main()
{
  test("Default options");
  test("Slash added", true);
  dfs.recurse = false;
  test("No recursion");
  dfs.includeDirs = true;
  test("Including dirs");
  dfs.prefix = "prefix_";
  test("With prefix");
  dfs.prefix = "my/prefix/";
  dfs.recurse = true;
  test("With everything", true);
  dfs.includeDirs = false;
  test("Removing dirs again");

  return 0;
}
