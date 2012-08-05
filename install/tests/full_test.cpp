#include "installer.hpp"
#include <iostream>

using namespace std;
using namespace Spread;

Hash hello("hello", 5);
Hash world("world", 5);
Hash testsh("ctEjJBRstghw4_UpmjBdhwJZFl8faISyIeEk2sOH5LLfAQ");
Hash zipfile("bJUN2VkwS-cRpIlPRil-yWIxuoOfiWUG4li9IxH5MGWkAQ");
Hash zipzip("RKEFhXsBxTYXmQt4oMxU_RQk_t7F0Oj8RMGALSJKh6UfAg");
Hash testzip("HjZUnmXcFYkS8mTsovU-oVKEdKoGbF7Ga3NlNWYX6spMAQ");
DirectoryPtr testDir, zipDir, zipzipDir;

Hash testDirH, zipDirH, zipzipDirH;

void setupDirs()
{
  testDir.reset(new Directory);
  zipDir.reset(new Directory);
  zipzipDir.reset(new Directory);

  testDir->dir["hello.out"] = hello;
  testDir->dir["dir/world.out"] = world;
  zipDir->dir["test.sh"] = testsh;
  zipzipDir->dir["testsh.zip"] = zipfile;

  using namespace Mangle::Stream;

  testDirH = testDir->write("_test.dir");
  zipDirH = zipDir->write("_zip.dir");
  zipzipDirH = zipzipDir->write("_zipzip.dir");
}

Cache::Cache cache;
RuleSet rules;

#define ADD(f,h) assert(cache.index.addFile(f)==(h))

void setup()
{
  setupDirs();

  cache.tmpDir = "_full_tmp/";

  ADD("_test.dir", testDirH);
  ADD("_zip.dir", zipDirH);
  ADD("_zipzip.dir", zipzipDirH);
  ADD("testdir.zip",testzip);
  ADD("zipzip.zip",zipzip);

  rules.addArchive(zipfile, zipDirH, "ZIP1");
  rules.addArchive(testzip, testDirH, "ZIP TEST");
  rules.addArchive(zipzip, zipzipDirH, "ZIPZIP");
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

#define TEST(msg,path) std::string _prefix="_full/"+std::string(path);Installer inst(cache, rules, _prefix);AdHoc _ad(inst,_prefix,(msg))

int main()
{
  setup();

  {
    TEST("Empty installer","t1");
  }

  {
    TEST("Requesting non-existing file","t2");
    inst.addDep("output.dat", hello);
  }

  {
    TEST("Requesting existing file","t3");
    inst.addDep("test.dir", testDirH);
  }

  {
    TEST("File from zip (no hint)", "t4");
    inst.addDep("test.zip", zipfile);
  }

  {
    TEST("File from zip (with hint)", "t5");
    inst.addDep("test.zip", zipfile);
    inst.addHint(zipzip);
  }

  {
    TEST("Repeating with no hint (using cache)", "t6");
    inst.addDep("test.zip", zipfile);
  }

  {
    TEST("Unpack two zips into one dir", "t7");
    inst.addDir(zipDirH);
    inst.addDir(zipzipDirH);
    inst.addDir(testDirH);
  }

  {
    TEST("Repeating previously non-existing file","t8");
    inst.addDep("output.dat", hello);
  }
  return 0;
}
