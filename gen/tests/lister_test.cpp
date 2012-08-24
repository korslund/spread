#include "packlister.hpp"
#include <iostream>

using namespace SpreadGen;
using namespace Spread;
using namespace std;

Cache::Cache cache;
RuleSet rules;

Hash dirHash;
Hash hello("hello",5);
Hash world("world",5);

Hash fakeArc("FAKE_ARC_HASH");
Hash fakeArcDir("FAKE_ARC_DIR");

Hash arcHash("ARC_WITH_DIR");

void genDir()
{
  Directory dir;
  dir.dir["hello.txt"] = hello;
  dir.dir["dir/world.txt"] = world;
  dirHash = dir.write("_lister_dir.out");
}

void print(const PackLister &lst)
{
  {
    std::map<std::string, PackLister::PackInfo>::const_iterator it;
    cout << "  PACKS:\n";
    for(it = lst.packs.begin(); it != lst.packs.end(); it++)
      {
        cout << "    " << it->first << ":\n";
        cout << "      Dirs:";
        for(int i=0; i<it->second.dirs.size(); i++)
          cout << " " << it->second.dirs[i];
        cout << "\n      Hints:";
        for(int i=0; i<it->second.hints.size(); i++)
          cout << " " << it->second.hints[i];
        cout << "\n      Version: " << it->second.version << endl;
      }
  }

  {
    cout << "  DIR-FILES:\n";
    PackLister::HashSet::const_iterator it;
    for(it = lst.dirs.begin(); it != lst.dirs.end(); it++)
      cout << "    " << *it << endl;
  }

  {
    cout << "  RULES:\n";
    RuleList::const_iterator it;
    for(it = lst.ruleSet.begin(); it != lst.ruleSet.end(); it++)
      cout << "    " << (*it)->ruleString << endl;
  }
  {
    cout << "  ARCHIVES:\n";
    std::set<const ArcRuleData*>::const_iterator it;
    for(it = lst.arcSet.begin(); it != lst.arcSet.end(); it++)
      cout << "    " << (*it)->ruleString << endl;
  }

  cout << endl;
}

void test1(const std::string &msg, bool hint=false)
{
  cout << msg << ":\n";

  PackLister lst(cache, rules);

  if(hint)
    {
      lst.addHint("test1", fakeArc);
      lst.setVersion("test1", "with-hint");
    }

  try { lst.addDir("test1", dirHash); }
  catch(exception &e)
    {
      cout << "ERROR: " << e.what() << "\n\n";
      return;
    }

  cout << "Dir FOUND\n";
  print(lst);
}

int main()
{
  genDir();

  test1("Non-existing dir");

  cache.index.addFile("_lister_dir.out");
  test1("Created dir file but nothing else");

  rules.addURL(hello, "url-to-hello");
  rules.addURL(hello, "other-hello-url", 3, 2);
  rules.addURL(Hash("blah"), "you will never see this");
  test1("Added rules for one of the files");

  test1("Testing with hint", true);
  rules.addArchive(fakeArc, fakeArcDir);
  test1("Added archive, but no hint");
  test1("Both archive and hint", true);

  rules.addArchive(arcHash, dirHash);
  test1("Testing with full archive");
  test1("Now with hint", true);
  
  rules.addURL(fakeArc, "fake-url", 12, 3.4);
  test1("Add an URL for the hint as well", true);

  rules.addURL(arcHash, "url-to-arc");
  test1("With URL rule for the full archive (no hint)");

  return 0;
}
