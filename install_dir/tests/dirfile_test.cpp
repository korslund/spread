#include "common.cpp"

Hash hello("HELLO");

struct MyTest : DirInstaller
{
  MyTest(const std::string &pref = "")
    : DirInstaller(own, rules, cache, pref) {}

  Hash::DirMap total;

  void test(const Hash &h, const std::string &path = "")
  {
    cout << "\nTEST(" << h << ", '" << path << "')\n";
    try
      {
        DirPtr p = addDirFile(total, h, path);
        print(*p, "LOADED");
      }
    catch(exception &e)
      {
        cout << "ERROR: " << e.what() << endl;
      }
    print(total, "TOTAL");
  }

  void doJob()
  {
    test(Hash());
    test(hello, "nowhere");

    Hash::DirMap dir1;
    dir1["file1"] = Hash("HASH1");
    dir1["file2"] = Hash("HASH2");

    Hash::DirMap dir2;
    dir2["file2"] = Hash("HASH3");
    dir2["file3"] = Hash("HASH4");

    Hash h1 = makeDir(dir1, "dirfile1.dat");
    Hash h2 = makeDir(dir2, "dirfile2.dat");

    test(h1);
    test(h2);
    test(h2, "sub/");
    test(h1, "sub/");

    setDone();
  }
};

int main()
{
  MyTest test("dirfile");
  test.run();
  return 0;
}
