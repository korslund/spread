#include "multi.hpp"

using namespace Tasks;
using namespace Jobs;

void MultiTask::doJob()
{
  // Run sub tasks
  JList::iterator it;
  for(it = joblist.begin(); it != joblist.end(); ++it)
    {
      Job &j = *it->first.get();
      bool useInfo = it->second;
      JobInfoPtr client = j.getInfo();

      if(runClient(j, useInfo))
        return;
    }

  // Everything is OK
  setDone();
}
