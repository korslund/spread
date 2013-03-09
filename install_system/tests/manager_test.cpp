#include "jobmanager.hpp"

#include <iostream>

using namespace std;
using namespace Spread;

Cache::Cache cache;
RuleSet rules;

int main()
{
  JobManager m(cache,rules);

  return 0;
}
