#include "filejob.hpp"
#include <iostream>
#include <rules/urlrule.hpp>
#include <rules/arcrule.hpp>
#include <install_system/hashfinder.hpp>

using namespace std;
using namespace Spread;

struct DummyCache : Cache::ICacheIndex
{
  map<Hash,string> files;

  int getStatus(const std::string &where, const Hash &hash)
  {
    using namespace Cache;
    if(files[hash] == where) return CI_Match;
    if(files[hash] != "") return CI_ElseWhere;
    return CI_None;
  }

  string findHash(const Hash &hash) { return files[hash]; }

  void addMany(const Hash::DirMap &dir)
  {
    assert(0);
  }

  Hash addFile(string,const Hash&) { assert(0); }
  void removeFile(const string&) { assert(0); }
  void getEntries(Cache::CIVector&) const { assert(0); }
};

struct DummyRules : RuleFinder
{
  const Rule* findRule(const Hash &hash) const
  {
    if(hash == file3)
      return new URLRule(hash, "RULESTR", "http://example.com/url");
    if(hash == file4)
      return new ArcRule(arcHash, DirectoryCPtr(new Directory), "RULESTR");
    return NULL;
  }

  void findAllRules(const Hash &hash, RuleList &output) const
  {
    assert(0);
  }

  void reportBrokenURL(const Hash &hash, const std::string &url)
  {
    cout << "Reporting broken URL=" << url << " hash=" << hash << endl;
  }
};

struct DummyOwner : FileJobOwner
{
  void notifyFiles(const Hash::DirMap &files)
  { assert(0); }

  std::string getTmpName(const Hash &hash)
  { return "tmp_" + hash.toString(); }

  bool getTarget(const Hash &hash, TargetPtr &job)
  {
    cout << "getTarget(" << hash << ")\n";
    return false;
  }

  MovableLock lock() { return MovableLock(); }
};

DummyRules rules;
DummyCache cache;
HashFinder fnd(rules, cache);
DummyOwner owner;
FileJob file(fnd, owner);

void test(const Hash &hash, const string &where="")
{
  cout << "\nSearching for HASH=" << hash << " FILE=" << where << endl;

  JobPtr job;
  string res;
  try { res = file.fetchFile(hash, job, where); }
  catch(std::exception &e)
    {
      cout << "ERROR: " << e.what() << endl;
      return;
    }
  cout << "  GOT: " << res << endl;

  if(job)
    {
      cout << "  JOB:\n";
      Target *t = dynamic_cast<Target*>(job.get());
      assert(t);
      const HashSource &out = t->src;

      cout << "    SRC: ";
      if(out.type == TST_File)
        cout << "Copy from " << out.value;
      else if(out.type == TST_Download)
        cout << "Download from " << out.value;
      else if(out.type == TST_Archive)
        cout << "Unpack from " << out.deps[0];
      else assert(0);
      cout << endl;

      cout << "    DST:\n";
      Hash::DirMap::const_iterator it;
      for(it = t->output.begin(); it != t->output.end(); it++)
        cout << "      " << it->second << " " << it->first << endl;

      cout << "  Running:\n";
      job.run();
    }
}

int main()
{
  test(file1);
  test(file2);
  test(file1, "blah");
  test(file1, "file2");
  test(file1, "file1");
  test(file2, "file1");
  test(file2, "file2");
  test(file3);
  test(file4);
  test(file5);

  file.brokenURL(file1, "hello");

  return 0;
}
