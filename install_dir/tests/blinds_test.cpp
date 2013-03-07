#include "common.cpp"

struct MyTest : DirInstaller
{
  MyTest(const std::string &pref = "")
    : DirInstaller(own, rules, cache, pref) {}

  void test(const std::string &s)
  {
    cout << "TESTING: " << s << endl;
    print(preBlinds, "PRE-BLINDS");
    print(postBlinds, "POST-BLINDS");
    preHash.clear();
    postHash.clear();

    try
      {
        sortBlinds();
        cout << "OUTPUT:\n";
        print(preHash, "PRE-HASH");
        print(postHash, "POST-HASH");
      }
    catch(exception &e)
      { cout << "ERROR: " << e.what() << endl; }
    resetAll();
    cout << endl;

    pre.clear();
    post.clear();
    preBlinds.clear();
    postBlinds.clear();
  }

  void doJob()
  {
    test("Nothing");

    postBlinds.insert(HDValue(Hash("ARC1"), ""));
    test("Single blind install");

    postBlinds.insert(HDValue(Hash("ARC2"), "a"));
    postBlinds.insert(HDValue(Hash("ARC3"), "b/"));
    post["blah"] = Hash("blah");
    test("Multi-blind install with paths");

    postBlinds.insert(HDValue(Hash("ARC1"), ""));
    pre["blah"] = Hash("blah");
    test("Single install, with upgrade info present");

    preBlinds.insert(HDValue(Hash("ARC1"), ""));
    preBlinds.insert(HDValue(Hash("ARC2"), "a"));
    postBlinds.insert(HDValue(Hash("ARC3"), "b/"));
    postBlinds.insert(HDValue(Hash("ARC4"), ""));
    test("Pre and post blinds");

    preBlinds.insert(HDValue(Hash("ARC1"), ""));
    preBlinds.insert(HDValue(Hash("ARC2"), "a"));
    test("Just pre blinds");

    assert(!checkStatus());
    setDone();
  }
};

int main()
{
  MyTest test("some/path/");
  test.run();
  return 0;
}
