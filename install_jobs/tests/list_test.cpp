#include "tlist.hpp"

#include <iostream>
#include <job/thread.hpp>

using namespace std;
using namespace Spread;

TargetList list;
bool locked = false;

struct LockJob : Job
{
  void doJob()
  {
    cout << "  LOCKING\n";
    MovableLock lock = list.lock();
    locked = true;
    Thread::sleep(1.5);
    cout << "  UNLOCKING\n";
    setDone();
  }
};

struct NoJob : Job
{
  void doJob()
  {
    cout << "Running nojob... thinking hard ... done!\n";
    setDone();
  }
};

Hash AAA("AAA"), BBB("BBB");

int main()
{
  JobPtr ptr(new NoJob);

  cout << "Insertion:\n";
  cout << list.fetchOrInsert(AAA, ptr) << endl;

  cout << "Fetch (with local lock):\n";
  {
    MovableLock lock = list.lock();
    JobPtr ptr2;
    cout << list.fetchOrInsert(AAA, ptr2) << endl;
    assert(ptr == ptr2);
  }

  cout << "Finishing:\n";
  ptr->run();

  cout << "Removing.\n";
  Hash::DirMap dir;
  dir["abc"] = AAA;
  list.remove(dir);

  cout << "Reinsertion:\n";
  ptr.reset(new LockJob);
  cout << list.fetchOrInsert(AAA, ptr) << endl;
  cout << "Running LockJob.\n";
  Thread::run(ptr);
  while(!locked) Thread::sleep(0.01);
  cout << "Removing entry (will wait for lock)\n";
  list.remove(dir);
  cout << "Entry removed.\n";

  return 0;
}
