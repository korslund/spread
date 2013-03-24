#include "common.cpp"

struct MyTest : DirInstaller
{
  MyTest(const std::string &pref = "")
    : DirInstaller(own, rules, cache, pref, true) {}

  void test(const Hash &dirHash, const std::string &path="")
  {
    cout << "TESTING " << dirHash << " path=" << path << endl;
    Hash::DirMap out;
    HashDir blinds;
    try
      {
        handleHash(out, dirHash, blinds, path);
        print(out,"OUTPUT");
        print(blinds,"BLINDS");
      }
    catch(exception &e)
      { cout << "ERROR: " << e.what() << endl; }
    resetAll();
    cout << endl;
  }

  void doJob()
  {
    Hash dirH("DIR"), arcH("ARC");

    test(dirH);

    rules.addArchive(arcH, dirH, "RULESTR");
    test(dirH);
    test(arcH);

    Hash::DirMap dir;
    dir["file1"] = Hash("FILE1");
    dir["sub/file1"] = Hash("FILE2");
    dirH = makeDir(dir, "dir.dat");
    test(dirH);
    test(dirH); // Test that our resetting works - this should fail

    // This won't find anything
    try {fetchFile(Hash("FILE1"));} catch(...) {}
    cout << endl;

    dirH = makeDir(dir, "dir.dat");
    rules.addArchive(arcH, dirH, "RULESTR");
    test(dirH, "somedir/");

    // This will find the archived file through the ArcRuleSet
    try {fetchFile(Hash("FILE1"));} catch(...) {}
    cout << endl;

    assert(!checkStatus());
    setDone();
  }
};

int main()
{
  MyTest test("handle");
  test.run();
  return 0;
}
