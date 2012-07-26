#include <iostream>
#include "install_finder.hpp"

using namespace std;
using namespace Spread;

Hash hello("hello",5);
Hash dolly("dolly",5);
Hash world("world",5);
Hash yeah("yeah!",5);
Hash inzip1("inzip1", 6);
Hash inzip2("inzip2", 6);
Hash zipfile("zipfile", 7);

struct MyFinder : RuleFinder
{
  Rule ziprule;
  Rule zipurl;

  MyFinder()
  {
    ziprule.addOut(inzip1);
    ziprule.addOut(inzip2);
    ziprule.addDep(zipfile);
    ziprule.ruleString = "ZIP rule";

    zipurl.addOut(zipfile);
    zipurl.ruleString = "URL for zipfile";
  }

  const Directory *findDir(const Hash &hash) const { return NULL; }
  const Rule *findRule(const Hash &hash) const
  {
    if(hash == inzip1 || hash == inzip2)
      return &ziprule;

    if(hash == zipfile)
      return &zipurl;

    return NULL;
  }
};

MyFinder rules;
Cache::CacheIndex cache;

void print(InstallFinder &inf)
{
  inf.output.clear();
  inf.perform();

  cout << "\nHandled " << inf.deps.size() << " input dependencies into "
       << inf.output.size() << " hashes:\n";
  InstallFinder::ActionMap::iterator it;
  for(it = inf.output.begin(); it != inf.output.end(); it++)
    {
      InstallFinder::Action &a = it->second;

      cout << it->first << " : " << a.action;
      if(a.rule) cout << " (" << a.rule->ruleString << ")";
      cout << endl;

      std::set<std::string>::iterator it2;
      for(it2 = a.destlist.begin(); it2 != a.destlist.end(); it2++)
        cout << "  => " << *it2 << endl;

      if(a.rule)
        {
          const Rule &r = *a.rule;

          for(int i=0; i<r.deps.size(); i++)
            cout << "  <= " << r.deps[i] << endl;
        }
    }
}

int main()
{
  InstallFinder inf(rules, cache);

  cout << "Hashes:\n";
  cout << hello << " = HELLO\n";
  cout << dolly << " = DOLLY\n";
  cout << world << " = WORLD\n";
  cout << yeah << " = YEAH!\n";
  cout << inzip1 << " = INZIP1\n";
  cout << inzip2 << " = INZIP2\n";
  cout << zipfile << " = ZIPFILE\n";

  cache.addFile("hello.txt");
  cache.addFile("dolly.txt");

  inf.addDep("_file1_hello", hello);
  inf.addDep("_file2_world", world);
  inf.addDep("", dolly);
  inf.addDep("", yeah);
  inf.addDep("", hello);
  inf.addDep("_file3_dolly", dolly);
  inf.addDep("_file4_dolly", dolly);
  inf.addDep("_file5_inzip1", inzip1);
  inf.addDep("_file6_inzip2", inzip2);

  print(inf);
  print(inf);

  cache.addFile("world.txt");

  print(inf);

  return 0;
}

  /*
    So how to organize the output:

    - I think the action list we're building now is good. If we add a
      deplist to Action (so that it depends on a set of other hashes),
      then we can easily build that into a tree later. We don't need
      any ordering info in this list itself. The lookup is more than
      enough.

      Also, the fact that something exists in the lookup doesn't mean
      it is required. For when adding an archive we add all of it, and
      we override URLs for example.

      To sort that out, we can do a second pass to construct the
      ordered list from base requirements (which we still have), and
      prune from there.

      Hmm, well actually, all the files we've requested initially will
      have destination files attached to them. Because all internal
      deps are fileless. So I guess just looping the output and
      checking whether or not it has any DESTINATIONS, is enough.

    - I suggest when producing, that we set up a list of produced
      outputs, or something that covers the wanted set. So when
      unpacking we add all the requested outputs to it, then do the
      unpacking, then remove or mark those somehow as done. And the
      cache takes care of everything regarding keeping track of
      existing data (we tmp-store everything for all major operations
      until the op is done). If we need to restart because of an error
      or broken rule or whatever, then all files already installed or
      found temporarily will work automatically because of the cache.
  */
