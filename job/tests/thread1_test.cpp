#include "thread.hpp"
#include <iostream>
#include <unistd.h>

using namespace Jobs;
using namespace std;

struct MyJob : Job
{
  void doJob()
  {
    setBusy("Sleeping for 5 seconds");

    setProgress(0,5);
    for(int i=0; i<5; i++)
      {
        setProgress(i);
        Thread::sleep(1);
      }
    setProgress(5);
    setDone();
  }
};

int main()
{
  Job *j1 = new MyJob, *j2 = new MyJob;
  JobInfoPtr i1 = j1->getInfo(), i2 = j2->getInfo();

  Thread::run(j1);
  Thread::run(j2);

  while(!i1->isFinished())
    {
      Thread::sleep(0.3);
      cout << "Status: " << i1->getMessage() << " (" << i1->getCurrent() << "/" << i1->getTotal() << ")\n";
    }

  return 0;
}
