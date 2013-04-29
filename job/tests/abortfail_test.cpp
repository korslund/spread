#include "job.hpp"
#include <iostream>

using namespace std;
using namespace Spread;

struct Test : Job
{
  void doJob()
  {
    info->abort();
    failError();
  }
};

int main()
{
  Test test;
  JobInfoPtr info = test.run();
  assert(info->isAbort());
  assert(!info->isError());
  cout << "Test OK\n";
  return 0;
}
