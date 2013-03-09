#include "common.cpp"

int i = 0;

struct Answer : AskHandle
{
  bool askWait(AskPtr ask)
  {
    StringAskPtr s = StringAsk::cast(ask);
    assert(s);
    s->select(i++ % s->options.size());
    return (i%6)==0;
  }
};

struct MyTest : DirInstaller
{
  MyTest(const std::string &pref = "")
    : DirInstaller(own, rules, cache, pref) {}

  void doJob()
  {
    cout << ask("What's for dinner?", "Chicken") << endl;
    cout << ask("3+5=", "2", "8", "3", "-14") << endl;
    ask("Eh?", "blah", "bleh", "bloh"); cout << endl;
    ask("Eh?", "blah", "bleh", "bloh"); cout << endl;
    ask("Eh?", "blah", "bleh", "bloh"); cout << endl;
    ask("Eh?", "blah", "bleh", "bloh"); cout << endl;

    cout << "Done\n";
    setDone();
  }
};

int main()
{
  MyTest test("ask");
  own.asker = new Answer;
  test.run();
  return 0;
}
