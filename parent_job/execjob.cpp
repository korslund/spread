#include "execjob.hpp"
#include <assert.h>
#include <stdexcept>

using namespace Spread;

bool ExecJob::execJob(JobPtr p, bool failOnError)
{
  assert(!jobs.size());
  jobs.insert(p);

  lastJob = p->getInfo();
  runClient(p, true, false);
  jobs.erase(p);
  done.push_back(p);

  if(failOnError && lastJob->isNonSuccess())
    {
      if(lastJob->isError()) throw std::runtime_error(lastJob->getMessage());
      else throw std::runtime_error("Client job aborted: " + lastJob->getMessage());
    }

  return lastJob->isSuccess();
}
