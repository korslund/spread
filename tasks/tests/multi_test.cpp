#include "multi.hpp"

#include <iostream>
using namespace std;
using namespace Tasks;
using namespace Jobs;

struct Dummy : Job
{
  int val;
  Dummy(int i) : val(i) {}

  void doJob()
  {
    setProgress(val,val);
    cout << val << endl;

    if(val == 24) setError("Value was 24, DO NOT LIKE");
    else setDone();
  }
};

void testStatus(Job &j)
{
  JobInfoPtr info = j.getInfo();

  if(info->isBusy()) cout << "Busy!";
  else if(!info->hasStarted()) cout << "Not started yet!";
  else if(info->isSuccess()) cout << "Success!";
  else if(info->isNonSuccess()) cout << "Failure: " << info->getMessage();
  cout << "  - progress " << info->getCurrent() << "/" << info->getTotal() << endl;
}

int main()
{
  {
    cout << "Dummy test:\n";
    Dummy job1(10);
    testStatus(job1);
    job1.run();
    testStatus(job1);
  }

  {
    cout << "\nEmpty multi test:\n";
    MultiTask mult;
    testStatus(mult);
    mult.run();
    testStatus(mult);
  }

  {
    cout << "\nOne dummy task:\n";
    MultiTask mult;
    testStatus(mult);
    mult.add(new Dummy(1));
    mult.run();
    testStatus(mult);
  }

  {
    cout << "\nPlenty dummy tasks:\n";
    MultiTask mult;
    testStatus(mult);
    for(int i=0; i<10; i++)
      mult.add(new Dummy(i));
    mult.run();
    testStatus(mult);
  }

  {
    cout << "\nFailure in the middle:\n";
    MultiTask mult;
    testStatus(mult);
    for(int i=0; i<10; i++)
      mult.add(new Dummy(i+19));
    mult.run();
    testStatus(mult);
  }

  {
    cout << "\nSetting useInfo=false on some jobs:\n";
    MultiTask mult;
    testStatus(mult);
    mult.add(new Dummy(10));
    mult.add(new Dummy(20), false);
    mult.run();
    testStatus(mult);
  }

  {
    cout << "\nSetting useInfo=false on ALL jobs:\n";
    MultiTask mult;
    testStatus(mult);
    mult.add(new Dummy(10), false);
    mult.add(new Dummy(20), false);
    mult.run();
    testStatus(mult);
  }

  return 0;
}
