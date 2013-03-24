#include "common.cpp"

struct MyTest : DirInstaller
{
  MyTest(const std::string &pref = "")
    : DirInstaller(own, rules, cache, pref, true) {}

  HashDir add, del;
  Hash::DirMap upgrade;

  void test(const std::string &w)
  {
    cout << "TESTING " << w << ":\n";
    print(add, "ADD");
    print(del, "DEL");
    print(upgrade, "UPGRADE");
    doLog = false;
    // NOTE: the 'false' means overwrite (don't ask) on conflict
    resolveConflicts(add, del, upgrade, false);
    doLog = true;
    cout << "---\n";
    print(add, "ADD");
    print(del, "DEL");
    add.clear();
    del.clear();
    upgrade.clear();
    resetAll();
    cout << endl;
  }

  void a(const std::string &file, const Hash &h)
  { add.insert(HDValue(h,file)); }

  void d(const std::string &file, const Hash &h)
  { del.insert(HDValue(h,file)); }

  void u(const std::string &file, const Hash &h, const Hash &old)
  {
    a(file,h);
    upgrade[file] = old;
  }

  void c(const std::string &file, const Hash &h)
  {
    cache.add(file, h);
  }

  void doJob()
  {
    test("Nothing");

    a("file", Hash("FILE"));
    test("Overwriting non-existing file");

    a("file", Hash("FILE"));
    c("file", Hash("FILE"));
    test("Overwriting identical file");

    d("file", Hash("FILE"));
    test("Deleting non-existing file");

    d("file", Hash("FILE"));
    c("file", Hash("FILE"));
    test("Deleting expected file");

    u("file", Hash("FILE"), Hash("FILE"));
    c("file", Hash("FILE"));
    test("Upgrading identical file");

    u("file", Hash("NEW"), Hash("OLD"));
    c("file", Hash("OLD"));
    test("Upgrading expected file");

    u("file", Hash("NEW"), Hash("OLD"));
    test("Upgrading missing file");

    a("file1", Hash("HASH1"));
    a("file2", Hash("HASH1"));
    a("file3", Hash("HASH3"));
    u("file4", Hash("HASH4"), Hash("OLD4"));
    u("file5", Hash("HASH1"), Hash("HASH1"));
    c("file2", Hash("HASH1"));
    c("file3", Hash("HASH3"));
    c("file4", Hash("OLD4"));
    c("file6", Hash("ABC"));
    d("file6", Hash("ABC"));
    d("file7", Hash("DEF"));
    c("NOT_RELEVANT", Hash("BLAHBLAH"));
    test("Multiple targets");

    cout << "\nSTARTING CONFLICT TESTS:\n\n";

    a("file", Hash("FILE"));
    c("file", Hash("OLD"));
    test("Overwriting unexpected file - OVERWRITE");

    u("file", Hash("FILE"), Hash("OLD"));
    c("file", Hash("WRONG"));
    test("Overwriting unexpected file - OVERWRITE");

    d("file", Hash("FILE"));
    c("file", Hash("WRONG"));
    test("Deleting wrong file - KILL!");

    assert(!checkStatus());
    setDone();
  }
};

int main()
{
  MyTest test("test");
  test.run();
  return 0;
}
