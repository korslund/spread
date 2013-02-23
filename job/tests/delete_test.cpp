#include "thread.hpp"
#include <iostream>
#include <stdexcept>

using namespace Spread;
using namespace std;

struct MyJob : Job
{
  ~MyJob() { cout << "Destroying object\n"; }

  void doJob()
  {
    cout << "Running job\n";
    setDone();
  }
};

int main()
{
  {
    cout << "\nDirect execution:\n";
    MyJob j;
    j.run();
    cout << "Going out of scope\n";
  }

  {
    cout << "\nThread::run() with async=false:\n";
    Thread::run(new MyJob, false);
    cout << "Going out of scope\n";
  }

  {
    cout << "\nThread::run(), waiting for thread:\n";
    Thread::run(new MyJob)->wait();
    cout << "Going out of scope\n";
  }

  {
    cout << "\nThread::run(JobPtr) with async=false:\n";
    JobPtr ptr(new MyJob);
    Thread::run(ptr, false);
    cout << "Going out of scope\n";
  }

  {
    cout << "\nThread::run(JobPtr), waiting for thread:\n";
    JobPtr ptr(new MyJob);
    Thread::run(ptr)->wait();
    cout << "Going out of scope\n";
  }

  cout << "\nDone\n";

  return 0;
}
