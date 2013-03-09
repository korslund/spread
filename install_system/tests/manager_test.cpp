#include "jobmanager.hpp"

#include <job/thread.hpp>
#include <iostream>

using namespace std;
using namespace Spread;

Cache::Cache cache;
RuleSet rules;

struct MyJob : Job
{
  string name;

  MyJob(const string &n) : name(n)
  { cout << "CREATED " << name << endl; }

  void doJob()
  {
    cout << "RUNNING " << name << endl;
    setDone();
  }
};

int main()
{
  cout << "Job adding:\n";
  {
    JobManager *m = new JobManager(cache,rules);
    JobPtr p(m);
    cout << "Adding:\n";
    m->add(new MyJob("A"));
    cout << "Starting:\n";
    Thread::run(p);
    Thread::sleep(0.5);
    cout << "Adding again:\n";
    m->add(new MyJob("B"));
    Thread::sleep(0.5);
    cout << "Done here\n";
  }

  return 0;
}
