#include "action_installer.hpp"
#include <iostream>
#include "rules/urlrule.hpp"
#include "rules/arcrule.hpp"

using namespace std;
using namespace Spread;

Hash hello("hello", 5);
Hash world("world", 5);
Hash robots("N0VT8AYfLEu2hFufocgj9ykAQoNEgcQwzLW7m1Tfc-cj");
Hash testsh("ctEjJBRstghw4_UpmjBdhwJZFl8faISyIeEk2sOH5LLfAQ");
Hash zipfile("bJUN2VkwS-cRpIlPRil-yWIxuoOfiWUG4li9IxH5MGWkAQ");
Hash zipzip("RKEFhXsBxTYXmQt4oMxU_RQk_t7F0Oj8RMGALSJKh6UfAg");

struct Dummy : ActionInstaller
{
  ActionMap acts;

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

void test(ActionInstaller &a, const std::string &msg = "Empty")
{
  cout << endl << msg << ":\n";
  Jobify::JobInfoPtr info = a.start(false);
  assert(info->isFinished());
  if(info->isError()) cout << "ERROR";
  else if(info->isSuccess()) cout << "DONE";
  else cout << "UNKNOWN STATUS";
  cout << ": status=" << info->message << " progress=" << info->getCurrent()
       << "/" << info->getTotal() << endl;
}

int main()
{
  Dummy dum;
  test(dum);

  Action &helloAct = dum.acts[hello];

  helloAct = Action("hello.txt");
  test(dum);
  helloAct = Action("world.txt");
  test(dum);

  helloAct = Action("world.txt", "_hello.out");
  test(dum, "Copying wrong file");

  helloAct = Action("hello.txt", "_hello.out");
  test(dum, "Copying right file");

  helloAct = Action("hello.txt");
  helloAct.addDest("_hello2.out");
  helloAct.addDest("_hello3.out");
  test(dum, "Multiple outputs");

  helloAct = Action("nofile");
  test(dum);

  helloAct = Action("nofile", "_fail1.txt");
  test(dum, "Non-existing file");

  URLRule rule1(hello, "test rule", "http://tiggit.net/robots.txt");
  helloAct = Action(&rule1, "_fail2.txt");
  test(dum, "Failed download");

  Action &roboAct = dum.acts[robots];
  roboAct = Action(&rule1, "_robots.txt");
  helloAct = Action("disable");

  // TODO: This should have given some progress numbers
  test(dum, "Successful download");
  roboAct = Action("disable");

  dum.acts[zipfile] = Action("testsh.zip");
  test(dum);

  DirectoryPtr dir(new Directory);
  dir->dir["test.sh"] = testsh;
  ArcRule ziprule(zipfile, dir, "test zip");
  dum.acts[testsh] = Action(&ziprule, "_testsh.out");
  dum.acts[testsh].addDest("_testsh2.out");
  test(dum, "File inside zip");

  DirectoryPtr dir2(new Directory);
  dir2->dir["testsh.zip"] = zipfile;
  ArcRule ziprule2(zipzip, dir2, "zip inside zip");
  dum.acts[zipzip] = Action("zipzip.zip");
  dum.acts[zipfile] = Action(&ziprule2);
  test(dum, "Zip inside zip");

  return 0;
}
