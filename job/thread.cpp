#include "thread.hpp"

#include <boost/thread.hpp>
#include "misc/tostr.hpp"
#include <assert.h>

using namespace Spread;

struct ThreadObj
{
  JobPtr j;
  void operator()()
  {
    j->run();
    assert(j->getInfo()->isFinished());
    j.reset();
  }
};

void Thread::sleep(double seconds)
{
  int msecs = (int)(seconds*1000000);
  boost::this_thread::sleep(boost::posix_time::microseconds(msecs));
}

std::string Thread::getId()
{
  return toStr(boost::this_thread::get_id());
}

JobInfoPtr Thread::run(Job *j, bool async)
{
  assert(j);
  return run(JobPtr(j), async);
}

JobInfoPtr Thread::run(JobPtr j, bool async)
{
  assert(j);
  ThreadObj to;
  JobInfoPtr info = j->getInfo();
  to.j = j;
  if(async) boost::thread trd(to);
  else to();
  return info;
}
