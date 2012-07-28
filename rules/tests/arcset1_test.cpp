#include "arcruleset.hpp"

#include <iostream>
using namespace std;
using namespace Spread;

ArcRuleSet rules;

Hash hello("HELLO");
Hash world("WORLD");

Hash arc1("ARC1"), arc2("ARC2");

std::string h(const Hash &hash)
{
  return hash.toString().substr(0,8);
}

void test(const Hash &hash, RuleFinder &rules)
{
  cout << h(hash) << " - ";
  const Rule *r = rules.findRule(hash);

  if(!r)
    {
      cout << "NOT FOUND\n";
      return;
    }

  cout << "Rule found:\n  Rule: \"" << r->ruleString << "\"\n";
  for(int i=0; i<r->deps.size(); i++)
    cout << "  <= " << h(r->deps[i]) << endl;
  for(int i=0; i<r->outputs.size(); i++)
    cout << "  => " << h(r->outputs[i]) << endl;

  if(r->type == RST_Archive)
    {
      const ArcRule *arc = ArcRule::get(r);
      cout << "  Produces " << arc->dir->dir.size() << " file(s)\n";
    }
  else if(r->type == RST_URL)
    {
      /*
      const URLRule *url = URLRule::get(r);
      assert(!url->isBroken);

      cout << "  URL: " << url->url
           << "\n  Priority: " << url->priority
           << "\n  Weight: " << url->weight
           << "\n\n";
      */
      cout << "  URL-Rule\n";
    }
  else assert(0);
}

void test(const Hash &hash)
{
  test(hash, rules);
}

void test()
{
  test(hello);
  test(world);
  test(arc1);
  cout << endl;
}

int main()
{
  test();

  cout << "Adding archive 1:\n";
  Directory dir1;
  dir1.dir["file1"] = hello;
  rules.addArchive(arc1, &dir1, "Rule1");
  test();

  cout << "Adding archive 2:\n";
  Directory dir2;
  dir2.dir["file2"] = hello;
  dir2.dir["file3"] = world;
  rules.addArchive(arc2, &dir2, "Rule2");
  test();

  cout << "Adding archive 3:\n";
  Directory dir3;
  dir3.dir["file4"] = world;
  rules.addArchive(arc1, &dir3, "Rule3");
  test();

  cout << "Testing wrapper:\n";
  ArcRuleSet rules2(&rules);
  test(hello, rules2);

  return 0;
}
