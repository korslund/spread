#include "parentjob.hpp"
#include <assert.h>

#define LOCK boost::lock_guard<boost::mutex> lock(mutex)

using namespace Spread;

void ParentJob::cleanup()
{
  JobSet::const_iterator it;
  LOCK;
  for(it = jobs.begin(); it != jobs.end(); it++)
    {
      const JobPtr &j = *it;
      assert(j);
      j->getInfo()->abort();
    }
}
