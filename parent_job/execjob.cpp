#include "execjob.hpp"
#include <assert.h>

using namespace Spread;

bool ExecJob::execJob(JobPtr p)
{
  assert(!jobs.size());
  jobs.insert(p);

  lastJob = p->getInfo();
  runClient(p, true, false);
  jobs.erase(p);
  done.push_back(p);

  return lastJob->isSuccess();
}
