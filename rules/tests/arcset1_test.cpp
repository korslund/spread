#include "common.cpp"
#include "arcruleset.hpp"

ArcRuleSet rules;

Hash hello("HELLO");
Hash world("WORLD");

Hash arc1("ARC1"), arc2("ARC2");

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
  DirPtr dir1(new Hash::DirMap);
  (*dir1)["file1"] = hello;
  rules.addArchive(arc1, Dir::hash(*dir1), dir1, "Rule1");
  test();

  cout << "Adding archive 2:\n";
  DirPtr dir2(new Hash::DirMap);
  (*dir2)["file2"] = hello;
  (*dir2)["file3"] = world;
  rules.addArchive(arc2, Dir::hash(*dir2), dir2, "Rule2");
  test();

  cout << "Adding archive 3:\n";
  DirPtr dir3(new Hash::DirMap);
  (*dir3)["file4"] = world;
  rules.addArchive(arc1, Dir::hash(*dir3), dir3, "Rule3");
  test();

  cout << "Testing wrapper:\n";
  ArcRuleSet rules2(&rules);
  test(hello, rules2);

  return 0;
}
