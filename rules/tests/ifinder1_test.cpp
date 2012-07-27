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
  ActionMap output;
  inf.perform(output);

  cout << "\nHandled " << inf.deps.size() << " input dependencies into "
       << output.size() << " hashes:\n";
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
