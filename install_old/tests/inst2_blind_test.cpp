#include "action_installer.hpp"
#include <iostream>
#include "rules/urlrule.hpp"
#include "rules/arcrule.hpp"
#include "print_dir.hpp"

using namespace std;
using namespace Spread;

ActionMap acts;

struct Dummy : ActionInstaller
{
  std::string brokenURL(const Hash &hash, const std::string &url)
  {
    assert(0);
    return "";
  }

  void getActions(ActionMap &out)
  {
    out = acts;
  }

  std::string getTmpFile(const Hash &h)
  {
    return "_tmp_hashes/" + h.toString();
  }

  void addToCache(const Hash::DirMap &list)
  {
    for(Hash::DirMap::const_iterator it = list.begin();
        it != list.end(); it++)
      cout << "Created: " << it->second << "  -  " << it->first << endl;
  }
};

void test(const std::string &msg = "Empty")
{
  Dummy a;
  cout << endl << msg << ":\n";
  JobInfoPtr info = a.run();
  assert(info->isFinished());
  if(info->isError()) cout << "ERROR";
  else if(info->isSuccess()) cout << "DONE";
  else cout << "UNKNOWN STATUS";
  cout << ": status=" << info->getMessage() << " progress=" << info->getCurrent()
       << "/" << info->getTotal() << endl;
}

int main()
{
  Hash testzip("HjZUnmXcFYkS8mTsovU-oVKEdKoGbF7Ga3NlNWYX6spMAQ");

  cout << "Setting up blind unpack of testdir.zip to _blind1/\n";
  acts[testzip] = Action("testdir.zip");
  ArcRule zip(testzip, "RULESTRING");
  acts[Hash("DUMMY")] = Action(&zip, "_blind1");
  test("Blind unpack");

  printDir("_blind1");

  return 0;
}
