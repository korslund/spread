#include <iostream>
#include "ruleset.hpp"

#include <map>

using namespace std;
using namespace Spread;

Hash hello("hello",5);
Hash dolly("dolly",5);
RuleSet rules;

std::string h(const Hash &hash)
{
  return hash.toString().substr(0,8);
}

void test(const Hash &hash)
{
  const ArcRuleData *ard = rules.findArchive(hash);
  if(ard)
    {
      cout << "Found: arc=" << h(ard->arcHash) << " dir=" << h(ard->dirHash)
           << "\nString: " << ard->ruleString << "\n";
    }
  else
    {
      cout << h(hash) << " NOT FOUND!\n";
    }
}

void test()
{
  test(hello);
  test(dolly);
  cout << endl;
}

int main()
{
  cout << "Shooting blanks:\n";
  test();
  cout << "Adding one archive:\n";
  rules.addArchive(hello, dolly);
  test();
  cout << "Adding another archive:\n";
  rules.addArchive(dolly, dolly, "DOLLEH!");
  test();

  return 0;
}
