#include <iostream>
#include "install_finder.hpp"

using namespace std;
using namespace Spread;

Hash zipfile("zipfile", 7);

struct MyFinder : RuleFinder
{
  Rule ziprule;
  Rule zipurl;

  MyFinder()
  {
    ziprule.addDep(zipfile);
    ziprule.ruleString = "ZIP rule";

    zipurl.addOut(zipfile);
    zipurl.ruleString = "URL for zipfile";
  }

  const Rule *findRule(const Hash &hash) const
  {
    if(hash == zipfile)
      return &zipurl;

    return NULL;
  }

  // Not used in this test
  void findAllRules(const Hash&, RuleList&) const { assert(0); }
};

MyFinder rules;
Cache::CacheIndex cache;

void print(InstallFinder &inf)
{
  ActionMap output;
  bool isOk = inf.perform(output);

  cout << "\nHandled " << inf.deps.size() << " input dependencies into "
       << output.size() << " hashes (OK=" << isOk << "):\n";
  ActionMap::iterator it;
  for(it = output.begin(); it != output.end(); it++)
    {
      Action &a = it->second;

      cout << it->first << " : ";

      if(a.isNone())
        {
          cout << "UNHANDLED\n";
        }
      else if(a.isRule())
        {
          const Rule &r = *a.rule;

          cout << r.ruleString << endl;

          for(int i=0; i<r.deps.size(); i++)
            cout << "  <= " << r.deps[i] << endl;
        }
      else if(a.isCopy())
        {
          cout << "COPY from " << a.source << endl;
        }
      else assert(0);

      std::set<std::string>::iterator it2;
      for(it2 = a.destlist.begin(); it2 != a.destlist.end(); it2++)
        cout << "  => " << *it2 << endl;
    }
}

int main()
{
  InstallFinder inf(rules, cache);
  inf.addBlind("some/destination/", zipfile);
  print(inf);

  return 0;
}
