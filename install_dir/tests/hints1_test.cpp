#include "dir_install.hpp"
#include <iostream>

using namespace Spread;
using namespace std;

struct MyCache : Cache::ICacheIndex
{
};

struct MyOwner : DirOwner
{
};

MyOwner own;
RuleSet rules;
MyCache cache;

struct MyTest : DirInstaller
{
  MyTest(const std::string &pref)
    : DirInstaller(own, rules, cache, pref) {}

  void doJob()
  {
    setDone();
  }
};

int main()
{
  return 0;
}
