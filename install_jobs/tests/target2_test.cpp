#include "target.hpp"
#include <boost/filesystem.hpp>
#include <iostream>
#include <job/thread.hpp>
#include "jobmaker.hpp"

using namespace std;
using namespace Spread;
namespace bf = boost::filesystem;

Hash hello("hello");
Hash world("world");
Hash fish("FISH");

/* This test makes fetchFile() return a job, and checks that Target
   runs it correctly (and fails if the job fails)
 */

struct TestJob : Job
{
  void doJob()
  {
    cout << "Running a test job!\n";
    Thread::sleep(0.5);
    cout << "DONE\n";
    setDone();
  }
};

struct TestJob2 : Job
{
  void doJob()
  {
    setError("This is supposed to fail");
  }
};

struct TestOwner : TargetOwner
{
  std::string fetchFile(const Hash &hash, JobPtr &job, const std::string &target)
  {
    assert(target == "");
    cout << "fetchFile(" << hash << ")\n";
    if(hash == world)
      {
        job.reset(new TestJob);
        return "Around the world";
      }
    if(hash == fish)
      {
        job.reset(new TestJob2);
        return "Do you like fishing?";
      }
    assert(0);
  }

  void brokenURL(const Hash &hash, const std::string &url)
  {
    cout << "Reporting broken URL=" << url << " HASH=" << hash << endl;
  }

  void notifyFiles(const Hash::DirMap &files)
  {
    cout << "Adding " << files.size() << " files to cache:\n";
    Hash::DirMap::const_iterator it;
    for(it = files.begin(); it != files.end(); it++)
    {
      const std::string &file = it->first;
      const Hash &hash = it->second;
      cout << "  " << hash << " " << file << endl;
    }
  }
};

TestOwner owner;
JobMaker maker;

void run(Job &j)
{
  cout << endl;
  JobInfoPtr info = j.run();
  cout << "  msg=" << info->getMessage()
       << "\n  prog=" << info->getCurrent() << "/" << info->getTotal()
       << endl;
  if(info->isSuccess()) cout << "  SUCCESS!\n";
  else if(info->isError()) cout << "  FAILURE!\n";
  else assert(0);
}

int main()
{
  bf::remove_all("_target2");
  {
    Target t(&owner, maker);
    t.src.type = TST_File;
    t.src.value = "hello.dat";
    t.src.hash = hello;
    // Add a fake dependencies to force fetchFile() to run
    t.src.deps.push_back(world);
    t.src.deps.push_back(fish);
    t.output["_target2/hello"] = hello;
    run(t);
  }

  return 0;
}
