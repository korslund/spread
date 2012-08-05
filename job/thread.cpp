#include "thread.hpp"

#include <boost/thread.hpp>
#include <assert.h>

using namespace Spread;

struct ThreadObj
{
  Job *j;
  void operator()()
  {
    j->run();
    assert(j->getInfo()->isFinished());
    delete j;
  }
};

void Thread::sleep(double seconds)
{
  int msecs = (int)(seconds*1000000);
  boost::this_thread::sleep(boost::posix_time::microseconds(msecs));
}

JobInfoPtr Thread::run(Job *j, bool async)
{
  assert(j);
  ThreadObj to;
  JobInfoPtr info = j->getInfo();
  to.j = j;
  if(async) boost::thread trd(to);
  else to();
  return info;
}
