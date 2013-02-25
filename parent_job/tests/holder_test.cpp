#include "jobholder.hpp"
#include "common.cpp"

struct TestJob : Job
{
  int i;
  TestJob(int i_) : i(i_) {}
  ~TestJob() { cout << "Deleting i=" << i << endl; }

  void doJob()
  {
    cout << "Running i=" << i << endl;

    if(i == 2) Thread::sleep(3);
    if(i == 3) throw runtime_error("Failing yo!");
    setDone();
  }
};

struct MyJobHolder : JobHolder
{
  void startup() { cout << "STARTUP\n"; }
  void tick() { cout << "TICK\n"; }
};

int main()
{
  JobHolder *jh = new MyJobHolder;
  JobPtr ptr(jh);

  Thread::run(ptr);
  print(ptr);
  cout << "Adding jobs:\n";
  jh->add(new TestJob(1));
  jh->add(new TestJob(2));
  jh->add(new TestJob(3));
  print(ptr);
  cout << "\nWaiting for finish:\n";
  jh->waitFinish();
  print(ptr);

  cout << "\nClearing failed list:\n";
  jh->clearFailed();
  print(ptr);

  cout << "EXIT\n";
  return 0;
}
