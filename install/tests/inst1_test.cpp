#include "action_installer.hpp"
#include <iostream>
#include "rules/urlrule.hpp"
#include "rules/arcrule.hpp"
#include "print_dir.hpp"

using namespace std;
using namespace Spread;

Hash hello("hello", 5);
Hash world("world", 5);
Hash robots("N0VT8AYfLEu2hFufocgj9ykAQoNEgcQwzLW7m1Tfc-cj");
Hash testsh("ctEjJBRstghw4_UpmjBdhwJZFl8faISyIeEk2sOH5LLfAQ");
Hash zipfile("bJUN2VkwS-cRpIlPRil-yWIxuoOfiWUG4li9IxH5MGWkAQ");
Hash zipzip("RKEFhXsBxTYXmQt4oMxU_RQk_t7F0Oj8RMGALSJKh6UfAg");

ActionMap acts;

struct Dummy : ActionInstaller
{
  std::string brokenURL(const Hash &hash, const std::string &url)
  {
    cout << "Broken url: " << url << endl;
    std::string backup = "hello-BACKUP-URL";
    if(hash == hello && url != backup)
      return backup;
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

  void addToCache(const Hash &h, const std::string &file)
  {
    cout << "Created: " << h << "  -  " << file << endl;
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
  test();

  Action &helloAct = acts[hello];

  helloAct = Action("hello.txt");
  test();
  helloAct = Action("world.txt");
  test();

  helloAct = Action("world.txt", "_hello.out");
  test("Copying wrong file");

  helloAct = Action("hello.txt", "_hello.out");
  test("Copying right file");

  helloAct = Action("hello.txt");
  helloAct.addDest("_hello2.out");
  helloAct.addDest("_hello3.out");
  test("Multiple outputs");

  helloAct = Action("nofile");
  test();

  helloAct = Action("nofile", "_fail1.txt");
  test("Non-existing file");

  URLRule rule1(hello, "test rule", "http://tiggit.net/robots.txt");
  helloAct = Action(&rule1, "_fail2.txt");
  test("Failed download");

  Action &roboAct = acts[robots];
  roboAct = Action(&rule1, "_robots.txt");
  helloAct = Action("disable");

  // TODO: This should have given some progress numbers
  test("Successful download");
  roboAct = Action("disable");

  acts[zipfile] = Action("testsh.zip");
  test();

  DirectoryPtr dir(new Directory);
  dir->dir["test.sh"] = testsh;
  ArcRule ziprule(zipfile, dir, "test zip");
  acts[testsh] = Action(&ziprule, "_testsh.out");
  acts[testsh].addDest("_testsh2.out");
  test("File inside zip");

  DirectoryPtr dir2(new Directory);
  dir2->dir["testsh.zip"] = zipfile;
  ArcRule ziprule2(zipzip, dir2, "zip inside zip");
  acts[zipzip] = Action("zipzip.zip");
  acts[zipfile] = Action(&ziprule2);
  test("Zip inside zip");

  return 0;
}
