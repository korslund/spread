#include <iostream>
#include <job/thread.hpp>
#include <assert.h>
#include <vector>
#include <set>
#include <sstream>
#include <boost/thread/mutex.hpp>
#include <queue>

template <typename X>
static std::string toStr(X i)
{
  std::stringstream str;
  str << i;
  return str.str();
}

using namespace std;

struct BranchJob;
typedef boost::shared_ptr<BranchJob> BranchPtr;

struct BranchJob : Spread::Job
{
  std::vector<BranchPtr> done;
  BranchPtr active;

  Spread::JobInfoPtr lastJob;

  virtual std::string getName() { return "Unknown"; }

  bool execJob(BranchPtr p)
  {
    assert(!active);
    active = p;

    lastJob = active->getInfo();
    runClient(active, true, false);

    done.push_back(active);
    active.reset();

    return lastJob->isSuccess();
  }

  /* Pointer version. Must be called with a newly created object that
     is not yet owned by a shared_ptr.
   */
  bool execJob(BranchJob *p)
  { return execJob(BranchPtr(p)); }

  /* TODO:
     - a system that auto-aborts children
     - a destructor that auto-aborts, and rule-of-three
     - two-way abortion detection (jobinfo <-> basejob)
     - use smart pointers for all children
   */
};

struct IntJob : BranchJob
{
  int i;

  IntJob(int _i) : i(_i) { cout << "Creating i = " << i << endl; }
  ~IntJob() { cout << "Destroying i = " << i << endl; }

  std::string getName() { return "IntJob=" + toStr(i); }

  void doJob()
  {
    setBusy("Converting " + toStr(i) + " => " + toStr(i*2));
    i *= 2;
    setProgress(i,i);

    if(i == 6) setError("Illegal value!");
    else setDone();
  }
};

struct WaitIntJob : BranchJob
{
  int lim, &ref;

  std::string getName() { return "WaitIntJob "+ toStr(lim); }

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
        Spread::Thread::sleep(0.02);
      }
    setProgress(ref, lim);

    if(ref == lim)
      setDone();
    else
      setError("Value is too high");
  }
};

#define LOCK boost::lock_guard<boost::mutex> lock(mutex)

template <class T>
class SafeQueue
{
  std::queue<T> queue;
  boost::mutex mutex;

public:

  void push(const T& t)
  {
    LOCK;
    queue.push(t);
  }

  bool pop(T &t)
  {
    LOCK;
    if(queue.empty()) return false;

    t = queue.front();
    queue.pop();
    return true;
  }
};

struct UserAsk
{
  std::string message;
  bool ready, abort;

  void abortJob() { abort = true; }

  UserAsk(const std::string &msg) : message(msg), ready(false), abort(false) {}
  virtual ~UserAsk() {}
};

struct StringAsk : UserAsk
{
  std::vector<std::string> options;
  int selection;

  void select(int i)
  {
    assert(i >= 0 && i < options.size());
    selection = i;
    ready = true;
  }

  StringAsk(const std::string &msg) : UserAsk(msg), selection(-1) {}

  static StringAsk* handle(UserAsk *ask) { return dynamic_cast<StringAsk*>(ask); }
};

SafeQueue<UserAsk*> g_askList;

struct AskJob : BranchJob
{
  std::string getName() { return "AskJob"; }

  void handleAsk(UserAsk &ask)
  {
    g_askList.push(&ask);
    while(!ask.ready)
      {
        Spread::Thread::sleep(0.02);

        if(ask.abort) info->abort();
        if(checkStatus()) return;
      }
  }

  int ask(const std::string &msg, const std::string &opt1="",
          const std::string &opt2="", const std::string &opt3="",
          const std::string &opt4="", const std::string &opt5="")
  {
    StringAsk ask(msg);

    // This is aweful code ;-)
    if(opt1 != "")
      {
        ask.options.push_back(opt1);
        if(opt2 != "")
          {
            ask.options.push_back(opt2);
            if(opt3 != "")
              {
                ask.options.push_back(opt3);
                if(opt4 != "")
                  {
                    ask.options.push_back(opt4);
                    if(opt5 != "")
                      ask.options.push_back(opt5);
                  }
              }
          }
      }

    handleAsk(ask);
    return ask.selection;
  }

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

struct AndJob : BranchJob
{
  std::string getName() { return "AndJob"; }

  std::vector<BranchPtr> jobs;

  void add(BranchPtr j) { jobs.push_back(j); }
  void add(BranchJob *j) { add(BranchPtr(j)); }

  void doJob()
  {
    // Start all the jobs
    for(int i=0; i<jobs.size(); i++)
      {
        const BranchPtr &j = jobs[i];
        assert(j);
        assert(!j->getInfo()->hasStarted());
        Spread::Thread::run(j);
      }

    // Regularly check up on the jobs
    while(true)
      {
        int64_t current = 0, total = 0;

        bool hasBusy = false;
        for(int i=0; i<jobs.size(); i++)
          {
            const BranchPtr &j = jobs[i];
            assert(j);
            Spread::JobInfoPtr inf = j->getInfo();

            current += inf->getCurrent();
            total += inf->getTotal();

            if(!inf->isFinished())
              hasBusy = true;
            else
              if(inf->isNonSuccess())
                setError("One or more child jobs finished unsuccessfully");
          }

        setProgress(current, total);

        // Exit if all jobs have finished, or if there was an error
        if(!hasBusy || checkStatus())
          break;

        Spread::Thread::sleep(0.1);
      }

    /* Move jobs over to 'done', and abort non-finished jobs. Note
       that calling getInfo()->abort() on an already completed job has
       no effect.
     */
    for(int i=0; i<jobs.size(); i++)
      {
        const BranchPtr &j = jobs[i];
        assert(j);
        j->getInfo()->abort();
        done.push_back(j);
      }
    jobs.clear();

    if(checkStatus()) return;
    setDone();
  }
};

BranchJob *tmp;
int g_value = 0;
int i1=0, i2=0, i3=0;

struct TestJob : BranchJob
{
  std::string getName() { return "TestJob"; }

  void doJob()
  {
    setBusy("Looping through tasks");

    std::vector<BranchPtr> list;

    list.push_back(BranchPtr(new IntJob(2)));
    list.push_back(BranchPtr(new IntJob(3)));
    list.push_back(BranchPtr(tmp = new WaitIntJob(g_value,10)));
    list.push_back(BranchPtr(new IntJob(4)));
    list.push_back(BranchPtr(new AskJob));

    AndJob *aj = new AndJob;
    aj->add(new WaitIntJob(i1));
    aj->add(new WaitIntJob(i2));
    aj->add(new WaitIntJob(i3));
    list.push_back(BranchPtr(aj));

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

void print(BranchPtr ptr, const std::string &name = "", int indent=0)
{
  cout << string(indent, ' ') << name;
  if(!ptr)
    {
      cout << "(null)\n";
      return;
    }

  Spread::JobInfoPtr inf = ptr->getInfo();
  if(inf->isSuccess()) cout << "SUC: ";
  else if(inf->isError()) cout << "ERR: ";
  else if(inf->isAbort()) cout << "ABO: ";
  else if(inf->isBusy()) cout << "BUS: ";
  else cout << "UNK: ";
  cout << ptr->getName() << ": ";
  if(inf->isSuccess()) cout << "Success";
  else cout << inf->getMessage();
  cout << " " << inf->getCurrent() << "/" << inf->getTotal();
  cout << endl;

  indent += 2;

  for(int i=0; i<ptr->done.size(); i++)
    print(ptr->done[i], "", indent);

  if(ptr->active)
    print(ptr->active, "ACT: ", indent);

  AndJob *aj = dynamic_cast<AndJob*>(ptr.get());
  if(aj)
    {
      for(int i=0; i<aj->jobs.size(); i++)
        print(aj->jobs[i], "", indent);
    }
}

int main()
{
  cout << "Start thread job and wait for CowJob:\n";
  BranchPtr job(new TestJob);
  Spread::Thread::run(job);
  while(!g_value) Spread::Thread::sleep(0.02);

  cout << "Status now:\n";
  print(job, "Main: ");

  cout << "\nSending the cows home, and waiting for finish\n";
  g_value = 10;
  //tmp->getInfo()->abort();
  //job->getInfo()->abort();

  // Wait until job is finished, or handle feedback requests along the
  // way
  Spread::JobInfoPtr it = job->getInfo();
  while(!it->isFinished())
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

              print(job, "Main: ");

              // All of these work correctly
              //ask->abortJob();
              //job->getInfo()->abort();
              //sask->select(1);
              sask->select(0);
            }
        }

      if(i1==1)
        {
          cout << "Finishing job I1\n";
          print(job, "Main: ");
          i1 = 2;
        }

      if(i2==1)
        {
          cout << "Finishing job I2\n";
          print(job, "Main: ");
          i2 = 2;
          Spread::Thread::sleep(0.1);
          continue;
        }

      if(i3==1)
        {
          cout << "Finishing job I3\n";
          print(job, "Main: ");
          i3 = 2;
        }

      Spread::Thread::sleep(0.05);
    }

  cout << "Job done. Status:\n";
  print(job, "Main: ");

  cout << "\nDeleting job\n";
  job.reset();
  cout << "EXIT\n";
  return 0;
}

/* THIS STUFF IS SIMPLE! It's not much work, and it fixes everything.

   - at this point, it's time to go through the old code

     - FIRST the old/starget stuff, I think we can kill all of that crap

     - then the old/jobs/target stuff, I think we have covered most of
       it here now. Check through it again carefully for anything that
       could affect our current code.

     - then anything else in old/ that's completely useless now

     - then finally the CURRENT install/action_installer
       implementation. Make sure we will be able to achieve EVERYTHING
       that it does the same or better. Look at EVERY single line of
       code and consider what special purpose it covers.

       Look ESPECIALLY at the passive unzip stuff, and figure out how
       we are going to handle that. It's pointless to create a new
       thread for every single file. At what point is a thread
       created?

       In general we do loads of stuff in the update thread in this
       solution. Figure out how that meshes with the current
       system. We should be FLEXIBLE enough to handle all kinds of use
       cases options!

   - expanded conditions:

     - a way to RETURN custom data, based on subclasses maybe. Can do
       a dynamic check to pick the right behavior, our string-ask list
       is then just one option of several. Make a function that
       handles them. Or maybe do StringAsk::handle(xyz) ||
       OtherAsk::handle(xyz) or similar.

     - most important part is to make sure we don't call any handle()s
       after we've dealt with the object, since it might be deleted at
       that point.

   - next: some structure that holds the main job, and possibly
     multiple jobs, that is always there and has all the maintenance
     stuff we need. This has a common base class with the AND object,
     but they differ in that the main thing doesn't abort if any of
     it's single clients fail. It's meant to be a permanent object,
     that you add stuff into continuously. Handles all the thread
     stuff internally, does progress summing and other required tasks.

     - this thing is the main interface for all inter-thread
       communication, and also between the main-thread and worker
       threads. So we need to include special functionality for this
       that has mutex locks etc. Not sure yet how to make this
       responsible for shared target hash completion data in a generic
       way. We could just do a subclass with base-class assisted mutex
       locking.

   - OR lists don't exist, they are implemented purely in code through
     execJob().
 */

/*
  Next go through and check that we have enough to handle this:

      PROCESS:
      ---------------------
      - input: pre and post arc hashes

      - looks up both to obtain dirhashes

      - find both dir objects (same process, called twice)
        - the pre hash SHOULD be stored locally, if it is already
          installed

        - arc case: post hash is obtained through downloading the new
          archive

        - patch case: dir object is found in a RULE package, which is
          a separate zip. This contains temporary rules (arc is
          downloaded to a tmp cache, rules are only inserted locally),
          and also the dir file and potentially bdiff files. All files
          are cached. All this is just loaded and remembered, it's not
          used at this point, except for the dir file.

        - dir file stored in permanent dir cache. This is part of the
          dir finding process, so BOTH dirs are stored.

       - compute the DIFF dir, which indicates what actions we are
         performing. Possible RESULT actions are:

         - add file hash,X (which means either add, overwrite or patch
           as possible)
         - delete file hash,X (must match the given hash)

         Through processing a third option is added:

         - move hash,X->Y, but ONLY if the cache confirms that X has
           the right hash. Otherwise, just leave the delete alone
           (error handling will take care of it below.)

       - add hints and arc rules:

         - the archive is still added as a source. The installer will
           be smart enough to prefer patching if possible.

         - all hints for the new dir are added to the tmp file rule
           holder. Since we have inserted the patch rules into this,
           then the patch rules for each individual file is inserted
           as well. Most likely, this ruleset includes all known patch
           rules for making the latest version (eg. 1.00->1.04,
           1.01->1.04 etc), but none of the bdiff files themselves.
           Which patches to download depends on our existing files.

         - we need to add arc rules for ALL the possible diffs to all
           new files. This doesn't mean downloading the archives, it's
           just necessary for the per-file installer system to find
           all the necessary files. The diff package rules should tell
           us everything we need to set up in order for this to work.

           This is looking more and more like a compiler->program
           relationship. Which is sort of what it IS, and is a great
           way to think about it actually. But it's a set of passive
           rules and hints rather than instructions, because the end
           result process is too environment dependent.

       - run the installer:

         - should just resolve everything itself, download extra patch
           archives and apply them, etc. How this needs to work is
           like this:

           - for each file, look up the rules. If we find a diff rule,
             then look up the pre-hash in our cache. Finds all cases,
             including possibly the file we are overwriting (the patch
             system backend handles that fine.)

           - then looks up the bdiff FILE, which we may or may not
             have. We need to have arc-loaded THIS file into the
             ruleset as well, just as all the individual destination
             files. IF we have done that, then the file is obtained
             normally through the get-archive system, and multiple
             files from the same archive are handled normally though
             the passive-wait deal, just as if we were installing
             normally.

           - if the bdiff rule was satisfied (all the targets wait for
             the archive to finish), then apply and be happy. If
             something did NOT work, the fallback system goes to the
             next option. Our system is robust at EVERY step.

             However, unlike our current system, we need LOGGING and
             transparency of all decisions made. It should be OBVIOUS
             what went wrong if something does go wrong.

             We ALSO have to put the user-action error handling system
             into all of this. Whenever there is a decision to be
             made, the thread needs the ability to PAUSE (enter a
             poll-mode, with the ability to abort completely if
             requested), and inform the top-level caller what has
             happened.

             Any actual abort or unrecoverable error causes the entire
             tree to abort and stop working, not just the current
             sub-thread. HOWEVER, an unrecoverable error in a leaf may
             be recoverable in the calling branch. We handled the
             total abort/exception thing in the existing system, so
             see what we did there.

      ---------------------

    - multiple zips (eg. screenshots)

      - the important difference here is the structure: the process
        above is called multiple times. But we use a SHARED data
        structure for storing repeated files, and never look up the
        same hash twice. This is similar to the list we already have
        in the existing solution.
*/

/*
struct InstallDir
{
  void fetchDir(Hash hash, Directory &dir)
  {
    // Check cache first
    // ...

    // Find the dirfile directly if possible
    execJob(new FetchFile(hash, whatever));
    // Check cache again

    // Does not return until finished. Is allowed to fail and this is
    // detected before we proceed.
    execJob(new GetArcDir(hash, dir));

    if(notFound)
      fail("Dir not found for " + hash.toString());

    // Store dir file in permanent dir cache
  }

  void doJob()
  {
    Directory dir1, dir2;

    fetchDir(hash1, dir1);
    if(hash2)
      fetchDir(hash2, dir2);

    // Get all hints associated with the dir hash
    processHints(rules.getHintsToDir(hash1, dir1));
    if(hash2)
      processHints(rules.getHintsFromDir(hash2, dir2));

    //- proceed with install based on dir
    //  - this may download the archive, or use existing files or
    //    other rules if preferred
    //  - existing non-matching files cause a recoverable error
  }
};
*/
