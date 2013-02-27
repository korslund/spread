#include "target.hpp"

/*
  There are only THREE STATES:

  - the file DOES NOT EXIST and nobody is getting it
  - the file IS BEING FETCHED - a target exists for it
  - the file EXISTS and is found through the cache

  We have to make sure the cache entry is written before the target
  entry is removed. The target list should ONLY ever list running
  jobs.
 */
/*
  This is used for blind unpacks, which are NOT handled in Target,
  because it's not a known target:

  new UnpackHash(outDir, outDirMap);

  We then need to store the outDirMap into the global dir cache as
  well, then set up our dirmap to include list all the output files
  converted to absolute paths, so our cache adding code below works
  correctly.
 */

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
  std::string fetchTmpFile(const Hash &hash, JobPtr &job)
  {
    cout << "fetchTmpFile(" << hash << ")\n";
    if(hash == zipH) return "test.zip";
    assert(0);
  }

  void brokenURL(const Hash &hash, const std::string &url)
  {
    cout << "Reporting broken URL=" << url << " HASH=" << hash << endl;
  }

  const Hash::DirMap &fetchArcDir(const Hash &hash)
  {
    cout << "fetchArcDir(" << hash << ")\n";
    assert(0);
  }

  void addToCache(const Hash::DirMap &files)
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
