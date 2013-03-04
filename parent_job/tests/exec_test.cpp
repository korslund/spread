#include "common.cpp"
#include "execjob.hpp"
#include <boost/thread/barrier.hpp>

std::vector<boost::barrier*> barriers;

void setup()
{
  barriers.resize(20);
  for(int i=0; i<barriers.size(); i++)
    barriers[i] = new boost::barrier(2);
}

void bwait(int i)
{
  assert(i>=0 && i<barriers.size());
  barriers[i]->wait();
}

struct TestJob : ExecJob
{
  int i;
  TestJob(int i_) : i(i_) {}

  void doJob()
  {
    cout << "start " << i << endl;

    if(i == 1)
      {
        setBusy("LUNCH BREAK");
        setProgress(3,4);
        cout << "Taking a break\n";
        bwait(2);
        bwait(3);
        cout << "Break's over, time to work\n";
      }

    if(i == 3)
      {
        // Abort ourselves
        info->abort();
        if(!checkStatus()) assert(0);
        return;
      }

    if(i == 4)
      {
        setProgress(5,6);
        throw runtime_error("I'm allergic to the number 4");
      }

    if(i == 5)
      {
        bwait(4);
        bwait(5);
        if(checkStatus())
          {
            cout << "Got ABORT in i=5 thread, exiting.\n";
            return;
          }
      }

    if(i == 0)
      {
        bwait(0);
        bwait(1);
        cout << "Returned from bwait\n";

        JobPtr j(new TestJob(1));
        execJob(new TestJob(2));
        execJob(j);
        assert(lastJob->isSuccess());
        cout << "Got: " << lastJob->getMessage() << endl;

        execJob(new TestJob(3), false);
        if(lastJob->isAbort())
          cout << "Got expected abort\n";
        else assert(0);

        execJob(new TestJob(4), false);
        if(lastJob->isError())
          cout << "Got expected error: " << lastJob->getMessage() << endl;
        else assert(0);

        cout << "Next job will abort us.\n";
        execJob(new TestJob(5), false);
        if(checkStatus())
          {
            cout << "Got ABORT in main job, exiting.\n";
            return;
          }
      }

    cout << "end " << i << endl;
    setDone();
  }
};

int main()
{
  setup();

  cout << "Staring main job\n";
  JobPtr ptr(new TestJob(0));
  JobInfoPtr info = Thread::run(ptr);
  bwait(0);
  cout << "Testing bwait\n";
  bwait(1);
  bwait(2);
  cout << "Back in MAIN, while the job is taking a break!\n";
  print(ptr);
  bwait(3);
  bwait(4);
  cout << "MAIN: Aborting caller thread now.\n";
  print(ptr);
  info->abort();
  cout << "Ok, returning.\n";
  bwait(5);

  info->wait();
  print(ptr);
  cout << "EXIT\n";
  return 0;
}
