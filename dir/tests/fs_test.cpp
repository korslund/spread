#include "print_dir.hpp"

using namespace std;
using namespace Spread;

Cache::CacheIndex index;

bool recurse = true;
bool includeDirs = false;
std::string prefix = "";

void test(const string &msg, bool addSlash = false)
{
  cout << endl << msg << endl;
  Hash::DirMap dir;
  if(addSlash) Dir::fromFS(dir, "testdir/", index, recurse, includeDirs, prefix);
  else Dir::fromFS(dir, "testdir", index, recurse, includeDirs, prefix);
  printDir(dir);
}

int main()
{
  test("Default options");
  test("Slash added", true);
  recurse = false;
  test("No recursion");
  includeDirs = true;
  test("Including dirs");
  prefix = "prefix_";
  test("With prefix");
  prefix = "my/prefix/";
  recurse = true;
  test("With everything", true);
  includeDirs = false;
  test("Removing dirs again");

  return 0;
}
