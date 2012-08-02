#include "notify.hpp"
#include <assert.h>

using namespace Tasks;
using namespace Jobs;

NotifyTask::NotifyTask(Jobs::Job *j)
  : other(j)
{
  assert(other);

  // Set client job of this one, to make sure info is passed along.
  setClient(other->getInfo());
}

void NotifyTask::doJob()
{
  other->run();
  if(clearClient()) return;
  setDone();
}

void NotifyTask::cleanup()
{
  if(info->isSuccess()) onSuccess();
  else onError();
}
