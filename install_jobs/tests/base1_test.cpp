#include "treebase.hpp"

#include <iostream>

using namespace std;
using namespace Spread;

Hash empty;
Hash nofile("nofile");
Hash hello("hello", 5), world("world", 5);

struct DummyFind : IHashFinder
{
  bool findHash(const Hash &hash, HashSource &out,
                const std::string &target="")
  {
    cout << "findHash(hash=" << hash << ", target=" << target << ")\n";
    out.hash = hash;
    out.type = TST_None;
    out.arcHash.clear();
    out.dirHash.clear();
    out.value.clear();
    out.deps.clear();

    if(hash == hello || hash == world)
      {
        out.type = TST_File;
        out.value = "cache/" + hash.toString().substr(0,4);
        if(target == out.value)
          out.type = TST_InPlace;
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
        cout << "  " << hash << " " << file << endl;
      }
  }
};

struct DummyLock
{
  DummyLock() { cout << "LOCKING\n"; }
  ~DummyLock() { cout << "UNLOCKING\n"; }
};

struct DummyTarget : TreeBase
{
  std::string what;
  HashDir outs;

  DummyTarget(TreeOwner &o, const std::string &w)
    : TreeBase(o), what(w) {}

  void addOutput(const Hash &h, const std::string &where)
  { outs.insert(HDValue(h,where)); }

  void doJob()
  {
    cout << "TARGET: " << what << endl;
    cout << "  Outputs:\n";
    HashDir::const_iterator it;
    Hash::DirMap dir;
    for(it = outs.begin(); it != outs.end(); it++)
      {
        cout << "    " << it->first << " " << it->second << endl;
        dir[it->second] = it->first;
      }
    finder.addToCache(dir);
    owner.notifyFiles(dir);
    setDone();
  }
};

struct DummyOwner : TreeOwner
{
  DummyOwner(IHashFinder &f) : TreeOwner(f) {}

  TreePtr target(const std::string &what)
  { return TreePtr(new DummyTarget(*this, what)); }

  TreePtr copyTarget(const std::string &from)
  { return target("COPY " + from); }

  TreePtr downloadTarget(const std::string &url)
  { return target("DOWNLOAD " + url); }

  TreePtr unpackTarget(const Hash &arc, const Hash &dir) {assert(0);}

  void notifyFiles(const Hash::DirMap &dir)
  {
    cout << "notifyFiles():\n";
    Hash::DirMap::const_iterator it;
    for(it = dir.begin(); it != dir.end(); it++)
      {
        const std::string &file = it->first;
        const Hash &hash = it->second;
        cout << "  " << hash << " " << file << endl;
      }
  }

  std::string getTmpName(const Hash &hash)
  { return "tmp_" + hash.toString(); }

  JobInfoPtr getRunningTarget(const Hash &hash)
  {
    cout << "getRunningTarget(" << hash << ")\n";
    return JobInfoPtr();
  }
  void setRunningTarget(const Hash &hash, JobInfoPtr ptr)
  {
    cout << "setRunningTarget(" << hash << ")\n";
  }

  Lock lock() { return Lock(new DummyLock); }
};

struct MyBase : TreeBase
{
  DummyFind fnd;
  DummyOwner own;

  MyBase() : own(fnd), TreeBase(own, fnd) {}

  // Circumvent protected function for testing purposes
  void fetch(const HashDir &outputs, HashMap *results = NULL)
  { fetchFiles(outputs, results); }

  void doJob() { assert(0); }
};

TreeBase::HashDir makeList;

void add(const Hash &h, const std::string &file="")
{
  makeList.insert(TreeBase::HDValue(h, file));
}

MyBase base;

void test()
{
  try
    {
      TreeBase::HashMap res;

      cout << "\nRUNNING fetchFiles():\n";
      base.fetch(makeList, &res);

      cout << "Results returned:\n";
      TreeBase::HashMap::const_iterator it;
      for(it = res.begin(); it != res.end(); it++)
        cout << it->first << " " << it->second << endl;
    }
  catch(std::exception &e)
    {
      cout << "ERROR: " << e.what() << endl;
    }

  makeList.clear();
}

int main()
{
  add(empty);
  test();

  add(nofile, "file");
  test();

  add(nofile);
  test();

  add(hello, "out_hello");
  add(hello, "out_hello2");
  add(world, "out_world");
  test();

  add(hello, "cache/LPJN");
  add(hello, "");
  add(world, "cache/SG6k");
  add(world, "");
  test();
  return 0;
}
