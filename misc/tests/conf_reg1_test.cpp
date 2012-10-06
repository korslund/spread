#include "jconfig.hpp"

#include <iostream>
#include <assert.h>
#include <boost/thread.hpp>

using namespace std;
using namespace Misc;

/* Regression test:

   JConfig was one of the first classes I wrote using boost::mutex,
   and it used manual lock() / unlock() instead of scoped lock guards.

   However this meant that an uncaught exception during a locked
   section would fail to unlock the mutex, causing the instance to
   deadlock and hang if another thread ever tried to use it.
 */
JConfig conf;

bool done = false;

struct ThreadObj
{
  void operator()()
  {
    cout << "Thread Start\n";
    conf.set("ghi", "jkl");
    cout << "Thread Done!\n";
    done = true;
  }
};

int main()
{

  cout << "Setting unusable file\n";
  try { conf.save("/"); }
  catch(...)
    { cout << "Got EXPECTED exception\n"; }

  cout << "Setting usable file\n";
  conf.save("_conf_reg1.conf");
  cout << "Setting value\n";
  conf.set("abc", "def");

  cout << "Setting value from another thread:\n";
  ThreadObj obj;
  boost::thread trd(obj);
  boost::this_thread::sleep(boost::posix_time::microseconds(400000));

  assert(done);

  cout << "Done\n";

  return 0;
}
