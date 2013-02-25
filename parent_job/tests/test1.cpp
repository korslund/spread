#include <iostream>
#include <job/thread.hpp>
#include <assert.h>
#include <vector>
#include <set>
#include <queue>

#include <sstream>
template <typename X>
static std::string toStr(X i)
{
  std::stringstream str;
  str << i;
  return str.str();
}

using namespace std;
using namespace Spread;

struct IntJob : Job
{
  int i;

  IntJob(int _i) : i(_i) { cout << "Creating i = " << i << endl; }
  ~IntJob() { cout << "Destroying i = " << i << endl; }

  void doJob()
  {
    setBusy("Converting " + toStr(i) + " => " + toStr(i*2));
    i *= 2;
    setProgress(i,i);

    if(i == 6) setError("Illegal value!");
    else setDone();
  }
};

struct WaitIntJob : Job
{
  int lim, &ref;

  WaitIntJob(int &r, int l=2) : lim(l), ref(r) {}

  void doJob()
  {
    setBusy("Waiting for the cows to come home");
    ref++;
    setProgress(ref, lim);
    while(ref < lim)
      {
        if(ref == -1) info->abort();
        if(checkStatus()) return;
        Thread::sleep(0.02);
      }
    setProgress(ref, lim);

    if(ref == lim)
      setDone();
    else
      setError("Value is too high");
  }
};

struct AskJob : Job
{
  void doJob()
  {
    setBusy("Asking the user something");
    int res = ask("What do you want for dinner?", "potatoes", "pie");
    if(checkStatus()) return;

    if(res == 0)
      {
        setProgress(4,4);
        setDone();
      }
    else if(res == 1)
      setError("We're out of pie!");
    else assert(0);
  }
};

Job *tmp;
int g_value = 0;
int i1=0, i2=0, i3=0;

struct TestJob : ExecJob
{
  void doJob()
  {
    setBusy("Looping through tasks");

    std::vector<JobPtr> list;

    list.push_back(JobPtr(new IntJob(2)));
    list.push_back(JobPtr(new IntJob(3)));
    list.push_back(JobPtr(tmp = new WaitIntJob(g_value,10)));
    list.push_back(JobPtr(new IntJob(4)));
    list.push_back(JobPtr(new AskJob));

    AndJob *aj = new AndJob;
    aj->add(new WaitIntJob(i1));
    aj->add(new WaitIntJob(i2));
    aj->add(new WaitIntJob(i3));
    list.push_back(JobPtr(aj));

    for(int i=0; i<list.size(); i++)
      {
        if(checkStatus()) return;

        bool b = execJob(list[i]);
        cout << i << "=";
        if(b) cout << "SUCCESS";
        else if(lastJob->isError()) cout << "ERROR";
        else if(lastJob->isAbort()) cout << "ABORT";
        else cout << "UNKOWN";
        cout << ": " << lastJob->getMessage() << endl;
      }

    if(checkStatus()) return;

    if(lastJob->isSuccess())
      setDone();
    else
      setError("Last job failed");
  }
};

void print(JobPtr ptr, const std::string &name = "", int indent=0)
{
  cout << string(indent, ' ') << name;
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

      for(int i=0; i<pj->done.size(); i++)
        print(pj->done[i], "", indent);

      JobSet::const_iterator it;
      for(it = pj->jobs.begin(); it != pj->jobs.end(); it++)
        print(*it, "", indent);
    }
}

JobHolder *holder;
JobPtr holdptr;

void print()
{
  print(holdptr, "Main: ");
}

int main()
{
  cout << "Start thread job and wait for CowJob:\n";
  holder = new JobHolder;
  holdptr.reset(holder);
  Thread::run(holdptr);
  holder->add(new TestJob);
  while(!g_value) Thread::sleep(0.02);

  cout << "Status now:\n";
  print();

  cout << "\nSending the cows home, and waiting for responses\n";
  g_value = 10;
  //tmp->getInfo()->abort();

  // Handle requests along the way
  while(true)
    {
      cout << "i1=" << i1 << " i2=" << i2 << " i3=" << i3 << endl;

      UserAsk *ask;
      if(g_askList.pop(ask))
        {
          StringAsk *sask = StringAsk::handle(ask);
          if(sask)
            {
              cout << "USER QUESTION: " << sask->message << endl;
              for(int i=0; i<sask->options.size(); i++)
                cout << "  " << i << ": " << sask->options[i] << endl;

              print();

              // All of these work correctly
              //ask->abortJob();
              //holder->getInfo()->abort();
              //sask->select(1);
              sask->select(0);
            }
        }

      if(i1==1)
        {
          cout << "Finishing job I1\n";
          print();
          i1 = 2;
        }

      if(i2==1)
        {
          cout << "Finishing job I2\n";
          print();
          i2 = 2;
          Thread::sleep(0.1);
          continue;
        }

      if(i3==1)
        {
          cout << "Finishing job I3\n";
          print();
          i3 = 2;
          break;
        }

      Thread::sleep(0.05);
    }

  cout << "Waiting for jobs to finish\n";
  holder->waitFinish();

  cout << "All jobs done. Status:\n";
  print();

  cout << "\nDeleting main job\n";
  holdptr.reset();
  cout << "EXIT\n";
  return 0;
}
