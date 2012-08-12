#include "common.cpp"
#include "ruleset.hpp"
#include "rule_loader.hpp"

RuleSet rules;

void test(const Hash &hash)
{
  cout << "Rule search:\n";
  test(hash, rules);

  cout << "Archive search:\n";
  const ArcRuleData *arc = rules.findArchive(hash);

  if(arc)
    cout << "  ARC arc=" << arc->arcHash << " dir=" << arc->dirHash << endl;
  else
    cout << "  Nothing\n";

  cout << "\n";
}

Hash aaa("aaaa");
Hash bbb("bbbb");
Hash ccc("cccc");
Hash ddd("dddd");

int main()
{
  addRule(rules, "URL aaaa 2 4.3 test-url");
  addRule(rules, "URL bbbb other-url");
  addRule(rules, "ARC bbbb cccc");

  test(aaa);
  test(bbb);
  test(ccc);
  test(ddd);

  loadRulesJsonFile(rules, "rules.json");

  test(aaa);
  test(bbb);
  test(ccc);
  test(ddd);

  return 0;
}
