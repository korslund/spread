#include "parentjob.hpp"
#include <iostream>
#include <job/thread.hpp>
#include <stdexcept>
#include <assert.h>

using namespace std;
using namespace Spread;

void print(JobPtr ptr, int indent=0)
{
  cout << string(indent, ' ');
  if(!ptr)
    {
      cout << "(null)\n";
      return;
    }

  JobInfoPtr inf = ptr->getInfo();
  if(inf->isSuccess()) cout << "SUC: ";
  else if(inf->isError()) cout << "ERR: ";
  else if(inf->isAbort()) cout << "ABO: ";
  else if(inf->isBusy()) cout << "BUS: ";
  else cout << "UNK: ";
  if(inf->isSuccess()) cout << "Success";
  else cout << inf->getMessage();
  cout << " " << inf->getCurrent() << "/" << inf->getTotal();
  cout << endl;

  indent += 2;

  ParentJob *pj = dynamic_cast<ParentJob*>(ptr.get());

  if(pj)
    {
      boost::lock_guard<boost::mutex> lock(pj->mutex);

      const ParentJob::JobSet &jobs = pj->getJobs();
      const ParentJob::JobVec &done = pj->getDone();

      if(done.size())
        {
          cout << string(indent, ' ') << "DONE:\n";
          for(int i=0; i<done.size(); i++)
            print(done[i], indent);
        }

      if(jobs.size())
        {
          cout << string(indent, ' ') << "JOBS:\n";
          ParentJob::JobSet::const_iterator it;
          for(it = jobs.begin(); it != jobs.end(); it++)
            print(*it, indent);
        }
    }
}
