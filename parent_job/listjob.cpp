#include "listjob.hpp"
#include <assert.h>
#include <job/thread.hpp>

#ifdef NEED_LOCKGUARD
#include <boost/thread/lock_guard.hpp>
#endif

#define LOCK boost::lock_guard<boost::mutex> lock(mutex)

using namespace Spread;

void ListJob::add(JobPtr j)
{
  LOCK;
  jobs.insert(j);
  assert(j);
  assert(!j->getInfo()->hasStarted());
  if(started) Thread::run(j);
}

void ListJob::start()
{
  assert(info->isBusy());
  if(started) return;

  LOCK;

  // Tell add() to start newly added jobs immediately
  started = true;

  // Start all jobs already in the list
  JobSet::const_iterator it;
  for(it = jobs.begin(); it != jobs.end(); it++)
    {
      const JobPtr &j = *it;
      assert(j);
      assert(!j->getInfo()->hasStarted());
      Thread::run(j);
    }
}
