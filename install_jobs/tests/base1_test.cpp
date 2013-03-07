#include "treebase.hpp"

#include <iostream>
#include <boost/thread/recursive_mutex.hpp>
#include <job/thread.hpp>

using namespace std;
using namespace Spread;

Hash empty;
Hash nofile("nofile");
Hash hello("hello", 5), world("world", 5);
Hash keiko("keiko", 5), postei("postei", 6);

Hash multiArc("ARCME");
Hash a1("A1"), a2("A2"), a3("A3");

float dlsleep = 0, arcsleep = 0;

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

    if(hash == hello || hash == world || hash == multiArc)
      {
        out.type = TST_File;
        out.value = "cache/" + hash.toString().substr(0,4);
        if(target == out.value)
          out.type = TST_InPlace;
        return true;
      }

    if(hash == keiko)
      {
        out.type = TST_Download;
        out.value = "url://SOME/URL/";
        return true;
      }

    if(hash == postei)
      {
        out.type = TST_Archive;
        out.dirHash = postei;
        out.deps.push_back(keiko);
        return true;
      }

    if(hash == a1 || hash == a2 || hash == a3)
      {
        out.type = TST_Archive;
        out.dirHash = postei;
        out.deps.push_back(multiArc);
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
    : TreeBase(o), what(w), type(t)
  {
    cout << "Creating target WHAT=" << what << endl;
  }

  void addOutput(const Hash &h, const std::string &where)
  { outs.insert(HDValue(h,where)); }
  void addInput(const Hash &h)
  { ins.insert(HDValue(h,"")); }

  void doJob()
  {
    assert(finder);
    if(type == 1 && dlsleep != 0)
      {
        Thread::sleep(dlsleep);
        cout << "-- end dlsleep\n";
      }
    if(type == 2 && arcsleep != 0)
      {
        Thread::sleep(arcsleep);
        cout << "-- end arcsleep\n";
      }

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

    // Cheat and set busy status even though we are not running.
    setBusy();
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

  add(keiko, "");
  test();

  add(keiko, "blah");
  test();

  add(keiko, "");
  add(keiko, "blah");
  test();

  add(postei, "postei");
  test();

  dlsleep = 0.1;
  add(keiko, "keiko");
  add(postei, "postei");
  test();

  dlsleep = 0;
  arcsleep = 0.1;
  add(keiko, "keiko");
  add(postei, "postei");
  test();

  arcsleep = 0;
  dlsleep = 0;

  add(a1, "__a1_file");
  add(a2, "__a2_file");
  add(a3, "__a3_file");
  add(a2, "__a2_file2");
  test();

  return 0;
}
