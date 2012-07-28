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
