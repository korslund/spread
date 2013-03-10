#include "jobmanager.hpp"
#include "print_dir.hpp"
#include <boost/filesystem.hpp>

using namespace std;
using namespace Spread;

Hash hello("hello",5);
Hash world("world",5);

std::string base;
Hash::DirMap dir1;

struct SetupTest
{
  SetupTest(const std::string &dir)
  {
    base = dir;
    assert(base != "");
    boost::filesystem::remove_all(base);

    dir1["abc"] = hello;
    dir1["def/ghi"] = hello;
    dir1["jkl/mno"] = world;
  }
  ~SetupTest()
  {
    if(boost::filesystem::exists(base))
      printDir(base);
  }
};

void print(JobPtr ptr, int indent=0);
void print(Job *ptr, int indent=0)
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

  ParentJob *pj = dynamic_cast<ParentJob*>(ptr);

  if(pj)
    {
      boost::lock_guard<boost::mutex> lock(pj->mutex);

      const ParentJob::JobSet &jobs = pj->getJobs();
      const ParentJob::JobVec &done = pj->getDone();

      if(done.size())
        {
          cout << string(indent, ' ') << "---\n";
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
void print(JobPtr ptr, int indent)
{
  print(ptr.get(), indent);
}

struct Setup : Installer
{
  RuleSet rules;
  Cache::Cache cache;
  JobManager m;

  string desc;

  InstallerPtr inst;

  Setup(const std::string &d) : m(cache,rules), desc(d)
  {
    assert(base != "");
    cache.tmpDir = base + "/tmp";
    cache.files.basedir = base + "/cache";
    m.setLogger(Misc::LogPtr(new Misc::Logger()), false);
    m.finish();
    inst = m.createInstaller(base);
    m.addInst(inst);
    cout << "\nSTART: " << desc << endl;
  }
  ~Setup()
  {
    m.run();

    StringAskPtr err = m.getNextError();
    while(err)
      {
        cout << "GOT ERROR: " << err->message << endl;
        err->abortJob();
        err = m.getNextError();
      }
    cout << "RESULT:\n";
    print(&m,2);
  }

  void addFile(const std::string &file, const Hash &hash)
  { inst->addFile(file, hash); }
  void remFile(const std::string &file, const Hash &hash)
  { inst->remFile(file, hash); }
  void addDir(const Hash::DirMap &dir, const std::string &path = "")
  { inst->addDir(dir, path); }
  void remDir(const Hash::DirMap &dir, const std::string &path = "")
  { inst->remDir(dir, path); }
  void addDir(const Hash &hash, const std::string &path = "")
  { inst->addDir(hash, path); }
  void remDir(const Hash &hash, const std::string &path = "")
  { inst->remDir(hash, path); }
  void addHint(const Hash &hint)
  { inst->addHint(hint); }

  Hash index(const std::string &file) { return cache.index.addFile(file); }
};

