#include "hashtask.hpp"
#include "job/thread.hpp"

#include <iostream>
#include <stdexcept>

#include "print_dir.hpp"

using namespace std;
using namespace Spread;
using namespace Mangle::Stream;

// Some predefined data and hashes to play with
string D1 = "hey\n";
string D2 = "yo man\n";
string D3 = "fish tastes like good\n";
string D4 = "this is not a string\n";
string D5 = "this is a bad attempt at quasi-intellectual existentialism with flat humor\n";

Hash H1(D1.c_str(), D1.size());
Hash H2(D2.c_str(), D2.size());
Hash H3(D3.c_str(), D3.size());
Hash H4(D4.c_str(), D4.size());
Hash H5(D5.c_str(), D5.size());

struct TestJob;
struct TestTask : HashTask
{
  TestJob *j;

  string outdir;

  TestTask(TestJob *job);
  Job *createJob();
};

struct TestJob : Job
{
  TestTask *task;

  void doJob()
  {
    setBusy("messing about");
    cout << endl;
    test();
    if(checkStatus()) return;
    setDone();
  }

  void write(const Hash &h, const std::string &str, bool wanted=true)
  {
    cout << "Writing " << h << " - '" << str.substr(0,str.size()-1) << "'";
    StreamPtr ptr = task->getOutStream(h);
    if(!ptr)
      {
        cout << " - SKIPPED\n";
        assert(!wanted);
        return;
      }
    cout << endl;
    ptr->write(str.c_str(), str.size());
    assert(wanted);
  }

  virtual void test() = 0;
};

TestTask::TestTask(TestJob *job)
  : j(job)
{ job->task = this; }

Job *TestTask::createJob() { return j; }

void testJob(TestJob *j)
{
  TestTask t(j);
  t.run();
  if(t.getInfo()->isError())
    cout << "CAUGHT: " << t.getInfo()->getMessage() << endl;

  if(t.outdir != "")
    printDir(t.outdir);
}

struct T1 : TestJob
{
  void test()
  {
    cout << "TEST1: Plain vanilla writing\n";
    task->addOutput(H1, "_ht_test1/d1.txt");
    task->addOutput(H2, "_ht_test1/d2.txt");
    write(H1, D1);
    write(H2, D2);

    task->outdir = "_ht_test1";
  }

  ~T1() { cout << "Jobs get deleted properly\n"; }
};

struct T2: TestJob
{
  void test()
  {
    cout << "TEST2: Writing same hash twice\n";
    task->addOutput(H1, "_ht_test2/d1.txt");
    write(H1, D1);
    write(H1, D2, false);

    task->outdir = "_ht_test2";
  }
};

struct T3: TestJob
{
  void test()
  {
    cout << "TEST3: Writing unwanted hashes\n";
    write(H3, D3, false);
    write(H4, D4, false);
  }
};

struct T4 : TestJob
{
  void test()
  {
    cout << "TEST4: One hash to multiple locations\n";
    task->addOutput(H1, "_ht_test4/d1.txt");
    task->addOutput(H1, "_ht_test4/d1_again.txt");
    task->addOutput(H1, "_ht_test4/subdir/d1_here_too.txt");
    write(H1, D1);
    write(H1, D1, false);
    write(H2, D2, false);

    task->outdir = "_ht_test4";
  }
};

struct T5 : TestJob
{
  void test()
  {
    cout << "TEST5: Wrong content\n";
    task->addOutput(H1, "_ht_test5/wrong.txt");
    write(H1, D4);
    task->outdir = "_ht_test5";
  }
};

struct T6 : TestJob
{
  void test()
  {
    cout << "TEST6: Throw up\n";
    throw runtime_error("Blaaargh!");
  }
};

struct T7 : TestJob
{
  void test()
  {
    cout << "TEST7: Fail respectfully\n";
    setError("Excuse me, can I borrow the restroom?");
  }
};

// Test 8 was made obsolete by interface changes

struct T9 : TestJob
{
  void test()
  {
    cout << "TEST9: Not generating the requested output\n";
    task->addOutput(H3, "_ht_test9/this_file_is_sorely_needed.txt");
    cout << " ... slacking off ... \n";
  }
};

struct T10 : TestJob
{
  void test()
  {
    cout << "TEST10: All-around test, with threading\n";
    task->addOutput(H1, "_ht_test10/dir/file1.txt");
    task->addOutput(H2, "_ht_test10/file2.txt");
    task->addOutput(H3, "_ht_test10/dir/file3.txt");
    task->addOutput(H1, "_ht_test10/file1_again.txt");
    task->addOutput(H4, "_ht_test10/file4.txt");
    write(H5, D5, false);
    write(H4, D4);
    write(H3, D3);
    write(H2, D2);
    write(H1, D1);
    write(H4, D4, false);
  }
};

int main()
{
  testJob(new T1);
  testJob(new T2);
  testJob(new T3);
  testJob(new T4);
  testJob(new T5);
  testJob(new T6);
  testJob(new T7);
  testJob(new T9);

  Job *job = new TestTask(new T10);

  JobInfoPtr info = job->getInfo();
  Thread::run(job);
  cout << "\nWating for job to finish...\n";
  while(!info->isFinished()) {}

  if(info->isError())
    cout << "CAUGHT2: " << info->getMessage() << endl;

  printDir("_ht_test10");

  return 0;
}
