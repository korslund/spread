#include "leaffactory.hpp"

#include <iostream>
#include <boost/filesystem.hpp>
#include <dir/binary.hpp>

using namespace std;
using namespace Spread;

namespace bf = boost::filesystem;

Hash hello("hello", 5);
Hash robot("N0VT8AYfLEu2hFufocgj9ykAQoNEgcQwzLW7m1Tfc-cj");
Hash testzip("UZuuyrHbX1c57drq6a6SObKZQrkr58Y09SrBFtEHm9rSAg");
Hash zipdir;
Hash dolly("rSdU-hHwettk1icc_gOrTKGJKe3BWeMSCHFkDKgmnf4M");

string dirFile = "_leaf1/dir1.dat";

bool enableFallback = false;
bool enableZip = false;

struct DummyFind : IHashFinder
{
  bool findHash(const Hash &hash, HashSource &out,
                const std::string &target="")
  {
    cout << "findHash(hash=" << hash << ", target=" << target << ")\n";
    out.hash = hash;
    out.type = TST_None;
    out.dirHash.clear();
    out.value.clear();
    out.deps.clear();

    if(enableFallback && hash == robot)
      {
        out.type = TST_Download;
        out.value = "http://tiggit.net/robots.txt";
        return true;
      }

    if(enableZip && hash == testzip)
      {
        out.type = TST_File;
        out.value = "test.zip";
        return true;
      }

    if(hash == zipdir && !zipdir.isNull() && dirFile != "")
      {
        out.type = TST_File;
        out.value = dirFile;
        return true;
      }

    return false;
  }

  void brokenURL(const Hash &hash, const std::string &url)
  {
    cout << "brokenURL(" << hash << ", " << url << ")\n";
  }

  void addToCache(const Hash::DirMap &dir)
  {
    cout << "Adding " << dir.size() << " files to cache:\n";
    Hash::DirMap::const_iterator it;
    for(it = dir.begin(); it != dir.end(); it++)
      {
        const std::string &file = it->first;
        const Hash &hash = it->second;
        assert(!hash.isNull());
        assert(file != "");
        cout << "  " << hash << " " << file << endl;
      }
  }
};

struct DummyOwner : TreeOwner
{
  LeafFactory fact;

  TreePtr copyTarget(const std::string &from)
  { return fact.copyTarget(*this, from); }

  TreePtr downloadTarget(const std::string &url)
  { return fact.downloadTarget(*this, url); }

  TreePtr unpackTarget(const Hash &dir)
  { return fact.unpackTarget(*this, dir); }

  TreePtr unpackBlindTarget(const std::string &where,
                            const Hash &dir = Hash())
  { return fact.unpackBlindTarget(*this, where, dir); }

  Lock lock() { return Lock(new int); }

  void log(const std::string &msg) { cout << "LOG: " << msg << endl; }

  void loadDir(const std::string &file, Hash::DirMap &output,
               const Hash &check = Hash())
  {
    cout << "loadDir(" << file << "):\n";
    Hash h2 = Dir::read(output, file);
    assert(check.isNull() || check == h2);
  }

  void storeDir(const Hash::DirMap &dir, const Hash &check = Hash())
  {
    cout << "storeDir():\n";
    Hash::DirMap::const_iterator it;
    for(it = dir.begin(); it != dir.end(); it++)
      {
        const std::string &file = it->first;
        const Hash &hash = it->second;
        cout << "  " << hash << " " << file << endl;
      }
    Hash h2 = Dir::write(dir, dirFile);
    cout << "  HASH: " << h2 << endl;
    assert(check.isNull() || check == h2);
    zipdir = h2;
  }

  void notifyFiles(const Hash::DirMap &dir)
  {
    cout << "notifyFiles():\n";
    Hash::DirMap::const_iterator it;
    for(it = dir.begin(); it != dir.end(); it++)
      {
        const std::string &file = it->first;
        const Hash &hash = it->second;
        cout << "  " << hash << endl;
      }
  }

  std::string getTmpName(const Hash &hash)
  { return "_leaf1/tmp_" + hash.toString().substr(0,6); }

  JobInfoPtr getRunningTarget(const Hash &hash)
  { return JobInfoPtr(); }

  void setRunningTarget(const Hash &hash, JobInfoPtr ptr)
  {}
};

DummyOwner own;
HashFinderPtr find(new DummyFind);

void test(TreePtr job)
{
  job->finder = ::find;
  JobInfoPtr info = job->run();
  if(info->isSuccess()) cout << "SUCCESS!\n";
  else cout << "ERROR: " << info->getMessage() << endl;
  cout << endl;
}

int main()
{
  bf::remove_all("_leaf1");

  {
    TreePtr job = own.copyTarget("nofile");
    job->addOutput(hello, "_leaf1/copy_fail1");
    test(job);
  }

  {
    TreePtr job = own.copyTarget("hello.dat");
    job->addOutput(robot, "_leaf1/copy_fail2");
    test(job);
  }

  {
    TreePtr job = own.copyTarget("hello.dat");
    job->addOutput(hello, "_leaf1/copy_hello1");
    job->addOutput(hello, "_leaf1/copy_hello2");
    test(job);
  }

  {
    TreePtr job = own.downloadTarget("url://broken");
    job->addOutput(robot, "_leaf1/url_fail1");
    test(job);
  }

  enableFallback = true;
  {
    TreePtr job = own.downloadTarget("url://broken");
    job->addOutput(robot, "_leaf1/url_robots1");
    test(job);
  }

  {
    TreePtr job = own.unpackBlindTarget("_leaf1/blind1");
    job->addInput(testzip);
    test(job);
  }

  enableZip = true;
  {
    TreePtr job = own.unpackBlindTarget("_leaf1/blind1");
    job->addInput(testzip);
    test(job);
  }

  {
    TreePtr job = own.unpackTarget(zipdir);
    job->addInput(testzip);
    job->addOutput(dolly, "_leaf1/zip_dolly1");
    job->addOutput(dolly, "_leaf1/zip_dolly2");
    test(job);
  }
  return 0;
}
