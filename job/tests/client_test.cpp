#include "job.hpp"

#include <iostream>
using namespace std;
using namespace Spread;

struct TestJob : Job
{
  int val;
  bool fail;

  TestJob(int _val, bool _fail)
    : val(_val), fail(_fail) {}

  void doJob()
  {
    cout << "Running job val=" << val << endl;
    setProgress(val/2, val);
    if(fail) setError("Error");
    else setDone();
  }
};

struct Test : Job
{
  int val;
  bool fail;
  bool abort;

  void doJob()
  {
    cout << "Outer job val=" << val << endl;
    TestJob tst(val, fail);
    if(abort) info->abort();
    if(runClient(tst)) return;
    setDone();
  }
};

int val=1;

void testStatus(Job &j)
{
  JobInfoPtr info = j.getInfo();

  if(info->isBusy()) cout << "Busy!";
  else if(!info->hasStarted()) cout << "Not started yet!";
  else if(info->isSuccess()) cout << "Success!";
  else if(info->isError()) cout << "Failure: " << info->getMessage();
  else if(info->isAbort()) cout << "Abort!";
  cout << "  - progress " << info->getCurrent() << "/" << info->getTotal() << endl;
}

void test(bool fail, bool abort)
{
  Test t;
  t.val = val++;
  t.fail=fail;
  t.abort=abort;
  t.run();
  testStatus(t);
}

int main()
{
  test(false, false);
  test(true, false);
  test(false, true);
  test(true, true);

  return 0;
}
