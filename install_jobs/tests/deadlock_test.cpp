#include "treebase.hpp"

#include <iostream>
#include <boost/thread/recursive_mutex.hpp>
#include <job/thread.hpp>

/* This test illustrates what happens when there are cycles in the
   ruleset, ie. rules that depend on themselves directly or indirectly.

   We have NOT implemented any kind of deadlock detection yet. The
   test is currently disabled because it locks up. We currently assume
   the ruleset to be acyclic. In fact it is probably better to test
   untrusted rulesets for cycles up front than to try to recover from
   them at runtime.
 */

using namespace std;
using namespace Spread;

Hash arc1("ARC1"), arc2("ARC2"), arc3("ARC3");

struct DummyFind : IHashFinder
{
  TreeBase::HashMap files;
  void reset() { files.clear(); }

  bool findHash(const Hash &hash, HashSource &out,
                const std::string &target="")
  {
    cout << "findHash(hash=" << hash << ", target=" << target << ")\n";
    out.hash = hash;
    out.type = TST_None;
    out.dirHash.clear();
    out.value.clear();
    out.deps.clear();

    if(files[hash] != "")
      {
        out.type = TST_File;
        out.value = files[hash];
        if(out.value == target)
          out.type = TST_InPlace;
        return true;
      }

    // Direct self dependence
    if(hash == arc1)
      {
        out.type = TST_Archive;
        out.dirHash = arc1;
        out.deps.push_back(arc1);
        return true;
      }

    // Indirect self dependence
    if(hash == arc2)
      {
        out.type = TST_Archive;
        out.dirHash = arc3;
        out.deps.push_back(arc3);
        return true;
      }

    if(hash == arc3)
      {
        out.type = TST_Archive;
        out.dirHash = arc1;
        out.deps.push_back(arc2);
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

        files[hash] = file;
      }
  }
};

struct DummyTarget : TreeBase
{
  std::string what;
  HashDir outs, ins;
  int type;

  DummyTarget(TreeOwner &o, const std::string &w, int t)
    : TreeBase(o), what(w), type(t) {}

  void addOutput(const Hash &h, const std::string &where)
  { outs.insert(HDValue(h,where)); }
  void addInput(const Hash &h)
  { ins.insert(HDValue(h,"")); }

  void doJob()
  {
    assert(finder);
    cout << "TARGET: " << what << endl;
    if(ins.size())
      {
        cout << "  Inputs:\n";
        {
          HashDir::const_iterator it;
          for(it = ins.begin(); it != ins.end(); it++)
            {
              const Hash &hash = it->first;
              std::string name = it->second;

              cout << "    " << hash << " " << name << endl;
            }
        }

        HashMap res;
        fetchFiles(ins, res);
        cout << "  Processed inputs:\n";
        {
          HashMap::const_iterator it;
          for(it = res.begin(); it != res.end(); it++)
            cout << "    " << it->first << " " << it->second << endl;
        }
      }
    if(outs.size())
      {
        cout << "  Outputs:\n";
        HashDir::const_iterator it;
        Hash::DirMap dir;
        for(it = outs.begin(); it != outs.end(); it++)
          {
            const Hash &hash = it->first;
            std::string name = it->second;

            cout << "    " << hash << " " << name << endl;
            if(name == "")
              name = owner.getTmpName(hash);
            dir[name] = hash;
          }
        finder->addToCache(dir);
        owner.notifyFiles(dir);
      }
    setDone();
  }
};

struct DummyOwner : TreeOwner
{
  TreePtr target(const std::string &what, int type)
  { return TreePtr(new DummyTarget(*this, what, type)); }

  TreePtr copyTarget(const std::string &from)
  { return target("COPY " + from, 0); }

  TreePtr downloadTarget(const std::string &url)
  { return target("DOWNLOAD " + url, 1); }

  TreePtr unpackTarget(const Hash &dir)
  { return target("UNPACK", 2); }

  TreePtr unpackBlindTarget(const string&, const Hash &)
  { assert(0); }

  boost::recursive_mutex mutex;
  Lock lock() { return Lock(new boost::lock_guard<boost::recursive_mutex>(mutex)); }

  void loadDir(const std::string &file, Hash::DirMap &output,
               const Hash &check = Hash())
  { assert(0); }
  void storeDir(const Hash::DirMap &dir, const Hash &check = Hash())
  { assert(0); }

  void notifyFiles(const Hash::DirMap &dir)
  {
    Lock l = lock();
    cout << "notifyFiles():\n";
    Hash::DirMap::const_iterator it;
    for(it = dir.begin(); it != dir.end(); it++)
      {
        const std::string &file = it->first;
        const Hash &hash = it->second;
        cout << "  " << hash << endl;
        stuff[hash] = JobInfoPtr();
      }
  }

  void log(const std::string &msg) { cout << "LOG: " << msg << endl; }

  std::map<Hash, JobInfoPtr> stuff;

  void reset()
  {
    stuff.clear();
  }

  std::string getTmpName(const Hash &hash)
  { return "tmp_" + hash.toString().substr(0,6); }

  JobInfoPtr getRunningTarget(const Hash &hash)
  {
    Lock l = lock();
    cout << "getRunningTarget(" << hash << ")\n";
    JobInfoPtr ptr = stuff[hash];
    if(ptr) cout << "  FOUND!\n";
    return ptr;
  }
  void setRunningTarget(const Hash &hash, JobInfoPtr ptr)
  {
    Lock l = lock();
    cout << "setRunningTarget(" << hash << ")\n";
    assert(ptr);
    stuff[hash] = ptr;
  }
};

struct MyBase : TreeBase
{
  DummyFind *fnd;
  DummyOwner own;

  MyBase() : TreeBase(own)
  {
    fnd = new DummyFind;
    finder.reset(fnd);
  }

  // Circumvent protected function for testing purposes
  void fetch(const HashDir &outputs, HashMap &results)
  { fetchFiles(outputs, results); }

  void doJob() { assert(0); }

  void reset() { fnd->reset(); own.reset(); }
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
      base.fetch(makeList, res);

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
  base.reset();
}

int main()
{
  cout << "This test is DISABLED\n";
  /*
  add(arc1, "arc1");
  test();

  add(arc2, "arc2");
  test();
  */
  return 0;
}
