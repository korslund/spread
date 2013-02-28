#include "target.hpp"
#include <boost/filesystem.hpp>
#include <iostream>

using namespace std;
using namespace Spread;
namespace bf = boost::filesystem;

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

TestOwner owner;

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
  bf::remove_all("_target");
  {
    Target t(&owner);
    t.src.type = TST_File;
    t.src.value = "hello.dat";
    t.src.hash = hello;
    t.output["_target/hello"] = hello;
    run(t);
  }
  {
    Target t(&owner);
    t.src.type = TST_File;
    t.src.value = "hello.dat";
    t.src.hash = hello;
    t.output["_target/hello_fail"] = world;
    run(t);
  }
  {
    Target t(&owner);
    t.src.type = TST_File;
    t.src.value = "world.dat";
    t.src.hash = world;
    t.output["_target/world1"] = world;
    t.output["_target/world2"] = world;
    run(t);
  }
  /* Uncomment to test downloads
  {
    Target t(&owner);
    t.src.type = TST_Download;
    t.src.value = "http://tiggit.net/robots.txt";
    t.src.hash = robots;
    t.output["_target/robo1"] = robots;
    run(t);
  }
  //*/
  {
    Target t(&owner);
    t.src.type = TST_Download;
    t.src.value = "http://doesnt/exist";
    t.src.hash = robots;
    t.output["_target/robo2"] = robots;
    run(t);
  }

  Hash::DirMap dir;
  dir["test.sh"] = testsh;
  dir["dir/dolly.txt"] = dolly;

  {
    Target t(&owner);
    t.src.type = TST_Archive;
    t.src.hash = testsh;
    t.src.dir = &dir;
    t.src.deps.push_back(zipH);
    t.output["_target/test"] = testsh;
    run(t);
  }

  return 0;
}
