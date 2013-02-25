#include "listjob.hpp"
#include "common.cpp"

struct ListTest : ListJob
{
  int i;

  ListTest(int _i) : i(_i) {}

  void doJob()
  {
    cout << "Starting list " << i << endl;
    start();
    Thread::sleep(0.5);
    setDone();
  }
};

int main()
{
  ListJob *lst = new ListTest(0);
  JobPtr ptr(lst);

  cout << "\nADDING BEFORE START:\n";
  lst->add(JobPtr(new ListTest(1)));
  lst->add(new ListTest(2));
  print(ptr);

  cout << "\nSTARTING NOW:\n";
  ptr->run();
  print(ptr);

  cout << "\nADDING MORE STUFF:\n";
  lst->add(JobPtr(new ListTest(3)));
  lst->add(new ListTest(4));
  Thread::sleep(1.0);
  print(ptr);

  return 0;
}
