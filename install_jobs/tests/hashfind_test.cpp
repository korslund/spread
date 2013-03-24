#include "hashfinder.hpp"
#include <iostream>

#include <rules/urlrule.hpp>
#include <rules/arcrule.hpp>

using namespace std;
using namespace Spread;

Hash
  file1("file1"), file2("file2"), file3("file3"),
  file4("file4"), file5("file5"), arcHash("archive");

struct DummyCache : Cache::ICacheIndex
{
  map<Hash,string> files;

  DummyCache()
  {
    files[file1] = "file1";
    files[file2] = "file2";
  }

  int getStatus(const std::string &where, const Hash &hash)
  {
    using namespace Cache;
    if(files[hash] == where) return CI_Match;
    if(files[hash] != "") return CI_ElseWhere;
    return CI_None;
  }

  string findHash(const Hash &hash) { return files[hash]; }

  Hash addFile(string,const Hash&,bool) { assert(0); }
  void addMany(const Hash::DirMap&, const Cache::StrSet&) { assert(0); }
  void checkMany(Hash::DirMap&) { assert(0); }
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
      return new ArcRule(arcHash, DirCPtr(new Hash::DirMap), arcHash, "RULESTR");
    return NULL;
  }

  void findAllRules(const Hash &hash, RuleList &output) const
  {
    assert(0);
  }

  const std::vector<Hash>* findHints(const Hash&) const
  {
    assert(0);
    return NULL;
  }

  void reportBrokenURL(const Hash &hash, const std::string &url)
  {
    cout << "Reporting broken URL=" << url << " hash=" << hash << endl;
  }
};

DummyCache cache;
HashFinder fnd(RuleFinderPtr(new DummyRules), cache);
IHashFinder &ifn = fnd;

void test(const Hash &hash, const string &where="")
{
  cout << "\nSearching for HASH=" << hash << " FILE=" << where << endl;
  HashSource out;
  ifn.findHash(hash, out, where);

  cout << "  ";
  if(out.type == TST_None)
    cout << "Not found";
  else if(out.type == TST_InPlace)
    cout << "File is in place at " << out.value;
  else if(out.type == TST_File)
    cout << "File exists at " << out.value;
  else if(out.type == TST_Download)
    cout << "File is at URL=" << out.value;
  else if(out.type == TST_Archive)
    cout << "File is in archive HASH=" << out.deps[0];
  else assert(0);
  cout << endl;
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
  return 0;
}
