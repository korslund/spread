#include "common.cpp"

struct MyTest : DirInstaller
{
  MyTest(const std::string &pref = "")
    : DirInstaller(own, rules, cache, pref) {}

  HashDir add, del;

  void test(const std::string &w)
  {
    StrMap moves;

    cout << "TESTING " << w << ":\n";
    print(add, "ADD");
    print(del, "DEL");
    findMoves(add, del, moves);
    cout << "---\n";
    print(add, "ADD");
    print(del, "DEL");
    print(moves, "MOVES");
    add.clear();
    del.clear();
    resetAll();
    cout << endl;
  }

  void a(const std::string &file, const Hash &h)
  { add.insert(HDValue(h,file)); }

  void d(const std::string &file, const Hash &h)
  { del.insert(HDValue(h,file)); }

  void doJob()
  {
    test("Empty");

    a("file1", Hash("FILE1"));
    a("file2", Hash("FILE2"));
    d("file3", Hash("FILE3"));
    d("file4", Hash("FILE4"));
    test("Non-matching entries");

    a("file1", Hash("FILE1"));
    a("file2", Hash("FILE2"));
    d("file3", Hash("FILE2"));
    d("file4", Hash("FILE4"));
    test("One matching entry");

    d("file1", Hash("FILE1"));
    d("file2", Hash("FILE1"));
    d("file3", Hash("FILE2"));
    d("file4", Hash("FILE2"));
    a("file5", Hash("FILE1"));
    a("file6", Hash("FILE2"));
    a("file7", Hash("FILE1"));
    test("Multiple options");

    d("file1", Hash("FILE1"));
    d("file2", Hash("FILE2"));
    d("file3", Hash("FILE3"));
    d("file4", Hash("FILE4"));
    a("file5", Hash("FILE3"));
    a("file6", Hash("FILE1"));
    a("file7", Hash("FILE4"));
    a("file8", Hash("FILE2"));
    test("All entries matching");

    // This would give undefined results in the real world
    d("file1", Hash("FILE1"));
    d("file2", Hash("FILE2"));
    d("file3", Hash("FILE3"));
    d("file4", Hash("FILE4"));
    a("file1", Hash("FILE3"));
    a("file2", Hash("FILE1"));
    a("file3", Hash("FILE4"));
    a("file4", Hash("FILE2"));
    test("Pandemonium!");

    setDone();
  }
};

int main()
{
  MyTest test("ask");
  test.run();
  return 0;
}
