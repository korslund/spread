#include "common.cpp"

struct MyTest : DirInstaller
{
  MyTest(const std::string &pref = "")
    : DirInstaller(own, rules, cache, pref, true) {}

  void test(const std::string &s)
  {
    cout << "TESTING: " << s << endl;
    print(pre, "PRE");
    print(post, "POST");

    HashDir add, del;
    Hash::DirMap upgrade;
    sortAddDel(add, del, upgrade);
    print(add, "ADD");
    print(del, "DEL");
    print(upgrade, "UPGRADE");
    pre.clear();
    post.clear();
    cout << endl;
  }

  void doJob()
  {
    test("Nothing");

    post["file1"] = Hash("HASH1");
    test("Single file add");

    pre["file1"] = Hash("HASH1");
    test("Delete single file");

    post["file1"] = Hash("HASH1");
    post["file2"] = Hash("HASH2");
    pre["file3"] = Hash("HASH3");
    pre["file4"] = Hash("HASH4");
    test("Multiple adds and deletes");

    pre["file"] = Hash("OLD");
    post["file"] = Hash("NEW");
    test("Upgrade file");

    pre["file"] = Hash("OLD");
    post["file"] = Hash("OLD");
    test("Ignore file");

    pre["file1"] = Hash("OLD1");
    pre["file2"] = Hash("OLD1");
    pre["file3"] = Hash("OLD3");
    pre["file4"] = Hash("OLD4");
    pre["file5"] = Hash("OLD5");
    pre["file6"] = Hash("OLD6");
    pre["ign"] = Hash("IGN");

    post["file2"] = Hash("NEW2");
    post["file3"] = Hash("NEW2");
    post["file4"] = Hash("OLD1");
    post["file5"] = Hash("OLD5");
    post["file7"] = Hash("NEW7");
    post["ign"] = Hash("IGN");
    test("Testing everything");

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
