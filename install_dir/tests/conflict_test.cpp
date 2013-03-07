#include "common.cpp"

int answer = -1;

struct Answer : AskHandle
{
  bool askWait(AskPtr ask)
  {
    StringAsk *s = StringAsk::handle(ask);
    assert(s);

    cout << "QUESTION: " << s->message << endl;

    if(answer < 0 || answer >= s->options.size())
      {
        cout << "ABORT!\n";
        s->abortJob();
        return true;
      }

    cout << "ANSWER: " << s->options[answer] << endl;

    s->select(answer);
    return false;
  }
};

struct MyTest : DirInstaller
{
  MyTest(const std::string &pref = "")
    : DirInstaller(own, rules, cache, pref) {}

  HashDir add, del;
  Hash::DirMap upgrade;

  void test(const std::string &w)
  {
    cout << "TESTING " << w << ":\n";
    if(answer >= 0) cout << "  Preselected answer is: " << answer << endl;
    print(add, "ADD");
    print(del, "DEL");
    print(upgrade, "UPGRADE");
    doLog = false;
    resolveConflicts(add, del, upgrade);
    doLog = true;
    cout << "---\n";
    print(add, "ADD");
    print(del, "DEL");
    add.clear();
    del.clear();
    upgrade.clear();
    resetAll();
    answer = -1;
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

    cout << "\nSTARTING USER-ASK TESTS:\n\n";

    a("file", Hash("FILE"));
    c("file", Hash("OLD"));
    answer = 2;
    test("Overwriting unexpected file - KEEP");

    a("file", Hash("FILE"));
    c("file", Hash("OLD"));
    answer = 1;
    test("Overwriting unexpected file - OVERWRITE");

    a("file", Hash("FILE"));
    c("file", Hash("OLD"));
    answer = 0;
    test("Overwriting unexpected file - OVERWRITE + BACKUP");

    u("file", Hash("FILE"), Hash("OLD"));
    c("file", Hash("WRONG"));
    answer = 2;
    test("Overwriting unexpected file - KEEP");

    u("file", Hash("FILE"), Hash("OLD"));
    c("file", Hash("WRONG"));
    answer = 1;
    test("Overwriting unexpected file - OVERWRITE");

    u("file", Hash("FILE"), Hash("OLD"));
    c("file", Hash("WRONG"));
    answer = 0;
    test("Overwriting unexpected file - OVERWRITE + BACKUP");

    d("file", Hash("FILE"));
    c("file", Hash("WRONG"));
    answer = 1;
    test("Deleting wrong file - KEEP");

    d("file", Hash("FILE"));
    c("file", Hash("WRONG"));
    answer = 0;
    test("Deleting wrong file - KILL!");

    assert(!checkStatus());
    setDone();
  }
};

int main()
{
  MyTest test("test");
  own.asker = new Answer;
  test.run();
  return 0;
}
