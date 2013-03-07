#include "common.cpp"

struct MyTest : DirInstaller
{
  MyTest(const std::string &pref = "")
    : DirInstaller(own, rules, cache, pref) {}

  void test()
  {
    cout << "TESTING INPUT:\n";
    print(preBlinds, "PRE-BLINDS");
    print(postBlinds, "POST-BLINDS");

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
  }

  void doJob()
  {
    test();
    assert(!checkStatus());
    setDone();
  }
};

int main()
{
  MyTest test;
  test.run();
  return 0;
}
