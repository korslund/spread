#include "dir_install.hpp"
#include <iostream>
#include <boost/thread/recursive_mutex.hpp>
#include <dir/binary.hpp>

using namespace Spread;
using namespace std;

struct MyCache : Cache::ICacheIndex
{
  map<Hash,string> files;

  int getStatus(const std::string &where, const Hash &hash)
  {
    using namespace Cache;
    if(files[hash] == where) return CI_Match;
    if(files[hash] != "") return CI_ElseWhere;
    return CI_None;
  }

  void addMany(const Hash::DirMap &dir)
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

  string findHash(const Hash &hash) { return files[hash]; }

  void add(const std::string &s, const Hash& h)
  { files[h] = s; }

  Hash addFile(string,const Hash&) { assert(0); }
  void removeFile(const string&) { assert(0); }
  void getEntries(Cache::CIVector&) const { assert(0); }

  void reset() { files.clear(); }
};

struct DummyTarget : TreeBase
{
  std::string what;
  HashDir outs, ins;
  int type;

  DummyTarget(TreeOwner &o, const std::string &w, int t)
    : TreeBase(o), what(w), type(t)
  { cout << "CREATING Target what=" << what << endl; }

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

struct MyOwner : DirOwner
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

  std::map<std::string, Hash::DirMap> dirs;

  void loadDir(const std::string &file, Hash::DirMap &output,
               const Hash &check = Hash())
  {
    cout << "loadDir(" << file << ")\n";
    if(dirs.find(file) == dirs.end())
      throw std::runtime_error("NO DIR FILE " + file);
    output = dirs[file];
    Hash h = Dir::hash(output);
    cout << "  DIR = " << h << endl;
    if(check.isOk() && check != h)
      throw std::runtime_error("HASH MISMATCH! dir=" + h.toString() +
                               " check=" + check.toString());
  }

  void storeDir(const Hash::DirMap &dir, const Hash &check = Hash())
  { assert(0); }

  void notifyFiles(const Hash::DirMap &dir)
  {
    Lock l = lock();
    cout << "notifyFiles():\n";
    Hash::DirMap::const_iterator it;
    for(it = dir.begin(); it != dir.end(); it++)
      {
        const Hash &hash = it->second;
        cout << "  " << hash << endl;
        stuff[hash] = JobInfoPtr();
      }
  }

  void log(const std::string &msg) { cout << "LOG: " << msg << endl; }

  std::map<Hash, JobInfoPtr> stuff;

  void reset()
  {
    dirs.clear();
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

  bool askWait(AskPtr ask, JobInfoPtr info) { assert(0); }
  void deleteFile(const std::string &path) { assert(0); }
 
};

void print(const Hash::DirMap &dir, const std::string &what)
{
  cout << what << ":\n";
  Hash::DirMap::const_iterator it;
  for(it = dir.begin(); it != dir.end(); it++)
    {
      const std::string &file = it->first;
      const Hash &hash = it->second;
      assert(file != "");
      cout << "  " << hash << " " << file << endl;
    }
}

void print(const TreeBase::HashDir &dir, const std::string &what)
{
  cout << what << ":\n";
  TreeBase::HashDir::const_iterator it;
  for(it = dir.begin(); it != dir.end(); it++)
    {
      const std::string &file = it->second;
      const Hash &hash = it->first;
      cout << "  " << hash << " " << file << endl;
    }
}

MyOwner own;
RuleSet rules;
MyCache cache;

Hash makeDir(const Hash::DirMap &dir, const std::string &filename)
{
  print(dir, "makeDir(): creating directory");
  Hash h = Dir::hash(dir);
  cache.add(filename, h);
  own.dirs[filename] = dir;
  cout << "  => " << filename << " " << h << endl;
  return h;
}

void resetAll()
{
  own.reset();
  cache.reset();
}
