#include <iostream>
#include "install_finder.hpp"
#include "ruleset.hpp"

using namespace std;
using namespace Spread;

Hash hello("hello",5);
Hash dolly("dolly",5);

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

RuleSet rules;
Cache::CacheIndex cache;

void cb(const Hash &h, const std::string &url)
{
  cout << "Broken URL " << url << endl;
}

int main()
{
  InstallFinder inf(rules, cache);
  rules.setURLCallback(&cb);

  cout << "Hashes:\n";
  cout << hello << " = HELLO\n";
  cout << dolly << " = DOLLY\n";

  inf.addDep("_file1_hello", hello);
  inf.addDep("_file2_dolly", dolly);
  print(inf);

  rules.addURL(hello, "hello-url");
  rules.reportBrokenURL(hello, "arne");
  print(inf);

  cache.addFile("hello.txt");
  rules.addURL(dolly, "dolly-url");
  print(inf);

  rules.reportBrokenURL(hello, "hello-url");
  rules.reportBrokenURL(hello, "dolly-url");
  print(inf);

  rules.reportBrokenURL(dolly, "dolly-url");
  print(inf);

  return 0;
}
