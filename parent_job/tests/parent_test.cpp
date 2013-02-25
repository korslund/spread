#include "common.cpp"

struct TestJob : ParentJob
{
  int i;

  TestJob(int _i) : i(_i) { cout << "Making i=" << i << endl; }
  ~TestJob() { cout << "Killing i=" << i << endl; }

  void doJob()
  {
    if(i==3) setError("Job3");
    if(i==4) throw runtime_error("Job4");
    if(i==20)
      {
        while(!checkStatus()) Thread::sleep(0.01);
      }
    if(i==10)
      {
        setBusy("Job10");
        jobs.insert(JobPtr(new TestJob(11)));
        jobs.insert(JobPtr(new TestJob(12)));
        done.push_back(JobPtr(new TestJob(1)));
        done.push_back(JobPtr(new TestJob(2)));
        done.push_back(JobPtr(new TestJob(3)));
        done.push_back(JobPtr(new TestJob(4)));

        done[0]->getInfo()->abort();
        Thread::run(done[0]);
        Thread::run(done[1]);
        Thread::run(done[2]);
        Thread::run(done[3]);
        Thread::sleep(0.3);

        // This job is auto-aborted when we exit
        JobPtr p(new TestJob(20));
        Thread::run(p);
        jobs.insert(p);
      }
    if(i==30)
      {
        while(!checkStatus())
          {
            cout << "Thread-unlocked\n";
            Thread::sleep(0.02);
            boost::lock_guard<boost::mutex> lock(mutex);
            cout << "Thread-locked\n";
            Thread::sleep(0.02);
          }
      }
    if(checkStatus()) return;
    setDone();
  }
};

int main()
{
  cout << "Hello\n";

  TestJob *t = new TestJob(10);
  JobPtr ptr(t);
  print(ptr);
  t->run();
  print(ptr);
  ptr.reset();
  t = new TestJob(30);
  ptr.reset(t);
  Thread::run(ptr);
  Thread::sleep(0.3);
  {
    boost::lock_guard<boost::mutex> lock(t->mutex);
    cout << "LOCKED!\n";
    Thread::sleep(0.5);
  }
  cout << "UNLOCKED\n";
  Thread::sleep(0.3);
  cout << "Aborting...\n";
  ptr->getInfo()->abort();
  cout << "Waiting...\n";
  ptr->getInfo()->wait();
  cout << "EXIT\n";

  return 0;
}
