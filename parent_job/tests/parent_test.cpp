#include <iostream>
#include "parentjob.hpp"
//#include "common.cpp"

using namespace std;
using namespace Spread;

/*
  Things to test:

  - that list inspection works
    - from both threads at the same time
    - we need a generalized lock-step mechanism for testing here, put
      one in common.cpp along with printing routines.

  - that locking works
    - do a fast lock/unlock loop in one thread, with output, then do a
      single long-lasting lock in the other, and see that output stops

  - that the correct jobs abort on exit
  - that aborting the testjob itself works
  - that exceptions work
  - that all jobs are deleted correctly when the job is destroyed

  By testing all these here, we don't have to re-test them in the
  subclasses.
 */

struct TestJob : ParentJob
{
  void doJob()
  {
  }
};

int main()
{
  cout << "Hello\n";

  return 0;
}
