#include "common.cpp"

Hash hello("HELLO");

struct MyTest : DirInstaller
{
  MyTest(const std::string &pref)
    : DirInstaller(own, rules, cache, pref) {}

  void doJob()
  {
    loadDirHints(hello);
    setDone();
  }
};

int main()
{
  /* TODO: There's a lot more to test here obviously. The hint rule
     system isn't done yet.
   */

  MyTest test("hint1");
  test.run();
  return 0;
}
