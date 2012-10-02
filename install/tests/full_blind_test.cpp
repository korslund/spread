#include "installer.hpp"
#include <iostream>
#include "print_dir.hpp"

using namespace std;
using namespace Spread;

Hash zipzip("RKEFhXsBxTYXmQt4oMxU_RQk_t7F0Oj8RMGALSJKh6UfAg");
Hash testzip("HjZUnmXcFYkS8mTsovU-oVKEdKoGbF7Ga3NlNWYX6spMAQ");
Hash zipfile("bJUN2VkwS-cRpIlPRil-yWIxuoOfiWUG4li9IxH5MGWkAQ");

Cache::Cache cache;
RuleSet rules;

#define ADD(f,h) assert(cache.index.addFile(f)==(h))

void setup()
{
  cache.tmpDir = "_blind_tmp/";

  ADD("testdir.zip",testzip);
  ADD("zipzip.zip",zipzip);
  rules.addArchive(testzip, Hash("NOT_USED"), "ARC: TESTZIP");
  rules.addArchive(zipzip, Hash("DUMMY_HASH"), "ARC: ZIPZIP");
  rules.addArchive(zipfile, Hash("NO_HASH"), "ARC: INNERZIP");
}

void test(ActionInstaller &a, const std::string &msg)
{
  cout << "\n" << msg << "\n";
  JobInfoPtr info = a.run();
  assert(info->isFinished());
  if(info->isError()) cout << "ERROR";
  else if(info->isSuccess()) cout << "DONE";
  else cout << "UNKNOWN STATUS";
  cout << ": status=" << info->getMessage() << "\nprogress=" << info->getCurrent()
       << "/" << info->getTotal() << endl;
}

struct AdHoc
{
  ActionInstaller &act;
  std::string prefix, msg;

  AdHoc(ActionInstaller &a, const std::string& pref, const std::string &m)
    : act(a), prefix(pref), msg(m) {}

  ~AdHoc() { test(act, msg + " => " + prefix + "/"); }
};

#define TEST(msg,path) std::string _prefix="_blind/"+std::string(path);Installer inst(cache, rules, _prefix);AdHoc _ad(inst,_prefix,(msg))

int main()
{
  cout << "Setup\n";
  setup();
  cout << "Setup done\n";

  {
    TEST("Unpacking zip 1","t1");
    inst.addDir(testzip);
  }

  {
    TEST("Unpacking zip 2","t2");
    inst.addDir(zipzip);
  }

  {
    TEST("Unpacking zip that was inside 2","t3");
    inst.addDir(zipfile);
  }

  printDir("_blind");
  return 0;
}
