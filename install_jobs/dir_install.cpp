#include "dir_install.hpp"
#include "hashfinder.hpp"
#include <rules/arcruleset.hpp>
#include <dir/tools.hpp>

using namespace Spread;

struct DirInstaller::_Internal
{
  RuleFinderPtr rulePtr;
  ArcRuleSet *rules;
  DirOwner *owner;
  std::string prefix;

  Hash::DirMap pre, post;
  TreeBase::HashDir preHash, postHash;
};

DirInstaller::DirInstaller(DirOwner &owner, RuleFinder &rules,
                           Cache::ICacheIndex &cache, const std::string &prefix)
  : TreeBase(owner)
{
  ptr.reset(new _Internal);
  ptr->rules = new ArcRuleSet(&rules);
  ptr->rulePtr.reset(ptr->rules);
  ptr->owner = &owner;
  ptr->prefix = prefix;

  finder.reset(new HashFinder(ptr->rulePtr, cache));
}

void DirInstaller::addFile(const std::string &file, const Hash &hash)
{
  assert(!getInfo()->hasStarted());
  ptr->pre[file] = hash;
}

void DirInstaller::remFile(const std::string &file, const Hash &hash)
{
  assert(!getInfo()->hasStarted());
  ptr->post[file] = hash;
}

void DirInstaller::addDir(const Hash::DirMap &dir, const std::string &path)
{
  assert(!getInfo()->hasStarted());
  Dir::add(ptr->pre, dir, path);
}

void DirInstaller::remDir(const Hash::DirMap &dir, const std::string &path)
{
  assert(!getInfo()->hasStarted());
  Dir::add(ptr->post, dir, path);
}

void DirInstaller::addDir(const Hash &hash, const std::string &path)
{
  assert(!getInfo()->hasStarted());
  ptr->preHash.insert(HDValue(hash,path));
}

void DirInstaller::remDir(const Hash &hash, const std::string &path)
{
  assert(!getInfo()->hasStarted());
  ptr->postHash.insert(HDValue(hash,path));
}

void DirInstaller::doJob()
{
  assert(0);
  setDone();
}
