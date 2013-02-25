#include <iostream>
#include "execjob.hpp"
//#include "common.cpp"

using namespace std;
using namespace Spread;

/*
  Things to test:

  - that both versions of execJob work
  - that lastJob is set correctly
  - that client success, errors, exceptions and aborts work as they
    should
  - that abort() requests are propagated to the client
  - that jobs are put in the correct list at all times
  - that we can continue running after a failed job, but also
    intercept it and choose to exit
  - that progress and status messages work
 */

int main()
{
  cout << "Hello\n";

  return 0;
}
