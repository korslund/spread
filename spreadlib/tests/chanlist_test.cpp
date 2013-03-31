#include <iostream>
#include "chanlist.hpp"
#include <boost/filesystem.hpp>

namespace bf = boost::filesystem;
using namespace Spread;
using namespace std;

void print(const PackInfo &pack)
{
  cout << "PACK: " << pack.package
       << "\nChannel: " << pack.channel
       << "\nVersion: " << pack.version
       << "\nSize: " << pack.installSize
       << "\nDirs:\n";
  assert(pack.dirs.size() == pack.paths.size());
  for(int i=0; i<pack.dirs.size(); i++)
    cout << "  " << pack.dirs[i] << "   " << pack.paths[i] << endl;
  cout << endl;
}

void print(const PackInfoList &list)
{
  cout << "ALL PACKS:\n";
  for(int i=0; i<list.size(); i++)
    print(list[i]);
}

void print(const PackList &list)
{ print(list.getList()); }

void print(const PackStatus &pack)
{
  cout << "PACK: " << pack.info.package
       << "\n  Channel: " << pack.info.channel
       << "\n  Version: " << pack.info.version
       << "\n  Size: " << pack.info.installSize
       << "\n  Dirs:\n";
  assert(pack.info.dirs.size() == pack.info.paths.size());
  for(int i=0; i<pack.info.dirs.size(); i++)
    cout << "    " << pack.info.dirs[i] << "   " << pack.info.paths[i] << endl;
  cout << "  STATUS: where=" << pack.where << " needsUpdate=" << pack.needsUpdate << "\n";
}

void print(const PackStatus *pack)
{
  if(pack) print(*pack);
  else cout << "NULL!\n";
}

void print(const PackStatusList &list)
{
  cout << "ALL ELEMENTS:\n";
  PackStatusList::const_iterator it;
  for(it = list.begin(); it != list.end(); it++)
    print(*it);
  cout << endl;
}

void print(const StatusList &list)
{
  PackStatusList out;
  list.getList(out);
  print(out);
}

RuleSet rules;
ChanList *ptr;

void print(const std::string &msg, const std::string &chn)
{
  cout << "\n=== " << msg << " (chan=" << chn << ") ===\n";
  print(ptr->getPackList(chn));
  print(ptr->getStatusList());
}

void print(const std::string &msg) { print(msg, "test2"); }

void install(const std::string &chn, const std::string &pack,
             const std::string &where)
{
  cout << "+ installing " << chn << "/" << pack << " => " << where << endl;
  ptr->getStatusList().setEntry(ptr->getPackList(chn).get(pack), where);
}

PackInfo pinf;

void installMod(const std::string &chn, const std::string &pack,
                const std::string &where)
{
  cout << "+ installing(mod) " << chn << "/" << pack << " => " << where << endl;
  pinf = ptr->getPackList(chn).get(pack);
  pinf.dirs.push_back("MOD");
  pinf.paths.push_back("mod/");
  ptr->getStatusList().setEntry(pinf, where);
  pinf.channel = "test3";
  ptr->getStatusList().setEntry(pinf, where);
  cout << "+ ditto for test3/\n";
}

void installLast()
{
  ptr->getStatusList().setEntry(pinf, "tmp/");
  cout << "+ reinserting fake entry/\n";
}

int main()
{
  bf::remove_all("data_chantest/installed.conf");
  bf::remove_all("data_chantest/installed.conf.old");
  ChanList chan("data_chantest", rules);
  ptr = &chan;

  // Missing rules.json
  print(chan.getStatusList());
  try { chan.getPackList("test0"); }
  catch(exception &e) { cout << "CAUGHT: " << e.what() << endl; }
  try { chan.load("test0"); }
  catch(exception &e) { cout << "CAUGHT: " << e.what() << endl; }

  // Missing packs.json
  try { chan.load("test1"); }
  catch(exception &e) { cout << "CAUGHT: " << e.what() << endl; }
  try { chan.load("test1"); }
  catch(exception &e) { cout << "CAUGHT: " << e.what() << endl; }

  print("LOADED");
  try { install("test2", "blah", "bleh"); }
  catch(exception &e) { cout << "CAUGHT: " << e.what() << endl; }
  install("test2", "pack2", "dir1/");
  install("test2", "pack2", "dir2/");
  print("INSTALLED");
  installMod("test2", "pack2", "dir2/");
  chan.load("test2"); // No effect
  print("INSTALLED MOD");

  JobInfoPtr info(new JobInfo);
  /* This has no effect, since the list doesn't exist yet. We will try
     loading whatever is on disk right now, since it can't be worse
     than just failing outright.
  */
  chan.setChannelJob("test3", info);

  /* At this point, installMod has inserted a package not matching
     what we are loading from disk. So the installed pack should now
     get needsUpdate set to true.
  */
  chan.load("test3");
  print("DATA MISMATCH");

  /* While 'info' is active though, 'test3' is blocked from reloading
     until the job has finished.
   */
  installLast();
  chan.load("test3");
  chan.load("test3");
  print("NOT RELOADING while BUSY");
  info->setError("Blah");
  chan.load("test3");
  print("NOT RELOADING while ERROR");
  chan.load("test3");
  info->setDone();
  print("RELOAD on SUCCESS");

  return 0;
}
