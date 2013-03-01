/* This test is identical to target1, except that it uses a dummy job
   creator instead of actual file system jobs.
 */

#include "target.hpp"
#include <iostream>
#include "ijobmaker.hpp"
#include <mangle/stream/servers/file_stream.hpp>
#include <mangle/stream/clients/copy_stream.hpp>

using namespace std;
using namespace Spread;

Hash hello("hello", 5);
Hash world("world", 5);
Hash robots("N0VT8AYfLEu2hFufocgj9ykAQoNEgcQwzLW7m1Tfc-cj");
Hash testsh("ctEjJBRstghw4_UpmjBdhwJZFl8faISyIeEk2sOH5LLfAQ");
Hash dolly("rSdU-hHwettk1icc_gOrTKGJKe3BWeMSCHFkDKgmnf4M");
Hash zipH("UZuuyrHbX1c57drq6a6SObKZQrkr58Y09SrBFtEHm9rSAg");

struct TestOwner : TargetOwner
{
  std::string fetchFile(const Hash &hash, JobPtr &job, const std::string &target)
  {
    assert(target == "");
    cout << "fetchFile(" << hash << ")\n";
    if(hash == zipH) return "test.zip";
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

struct DummyTask : HashTaskBase
{
  typedef HashTaskBase::HashDir::const_iterator HDI;
  std::string name;
  DummyTask(const std::string &nm) : name(nm) {}

  void doJob()
  {
    cout << "JOB " << name << ":\n";
    cout << "Producing " << outputs.size() << " element(s):\n";

    HDI it;
    for(it = outputs.begin(); it != outputs.end(); it++)
      cout << "  " << it->first << " " << it->second << endl;
    setDone();
  }
};

struct DummyMaker : IJobMaker
{
  HashTaskBase* copyJob(const std::string &from)
  {
    return new DummyTask("COPY " + from);
  }

  HashTaskBase* downloadJob(const std::string &url)
  {
    return new DummyTask("DOWNLOAD " + url);
  }

  HashTaskBase* unpackJob(const Hash::DirMap &index)
  {
    return new DummyTask("UNPACK");
  }
};

TestOwner owner;
DummyMaker maker;

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
  {
    Target t(&owner, maker);
    t.src.type = TST_File;
    t.src.value = "hello.dat";
    t.src.hash = hello;
    t.output["_target3/hello"] = hello;
    run(t);
  }
  {
    Target t(&owner, maker);
    t.src.type = TST_File;
    t.src.value = "world.dat";
    t.src.hash = world;
    t.output["_target3/world1"] = world;
    t.output["_target3/world2"] = world;
    run(t);
  }
  {
    Target t(&owner, maker);
    t.src.type = TST_Download;
    t.src.value = "http://tiggit.net/robots.txt";
    t.src.hash = robots;
    t.output["_target3/robo1"] = robots;
    run(t);
  }

  Hash::DirMap dir;
  dir["test.sh"] = testsh;
  dir["dir/dolly.txt"] = dolly;

  {
    Target t(&owner, maker);
    t.src.type = TST_Archive;
    t.src.hash = testsh;
    t.src.dir = &dir;
    t.src.deps.push_back(zipH);
    t.output["_target3/test"] = testsh;
    run(t);
  }

  return 0;
}
