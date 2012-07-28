#include "arcruleset.hpp"
#include <map>
#include <assert.h>

using namespace Spread;

typedef boost::shared_ptr<ArcRule> ArcPtr;
typedef std::map<Hash, ArcPtr> AMap;

struct ArcRuleSet::_Internal
{
  AMap files;

  const ArcRule* findFile(const Hash &hash)
  {
    AMap::iterator it = files.find(hash);
    if(it == files.end())
      return NULL;

    return it->second.get();
  }
};

const ArcRule *ArcRule::get(const Rule *r)
{
  assert(r->type == RST_Archive);
  const ArcRule *ur = dynamic_cast<const ArcRule*>(r);
  assert(ur != NULL);
  return ur;
}

ArcRuleSet::ArcRuleSet(RuleFinder *_base)
  : base(_base)
{
  ptr.reset(new _Internal);
}

const Rule *ArcRuleSet::findRule(const Hash &hash) const
{
  const ArcRule *r = ptr->findFile(hash);
  if(r) return r;

  // Fall back to base if no archives were found.
  if(base)
    return base->findRule(hash);

  return NULL;
}

void ArcRuleSet::addArchive(const Hash arcHash, const Directory *dir,
                            const std::string &ruleString)
{
  ArcPtr arcPtr(new ArcRule(arcHash, dir, ruleString));

  /* Loop through all the hashes in the directory, and point each of
     them to this archive.
  */
  Directory::DirMap::const_iterator it;
  for(it = dir->dir.begin(); it != dir->dir.end(); it++)
    {
      const Hash &fileHash = it->second;
      ptr->files[fileHash] = arcPtr;

      // Also add the output to the archive rule itself
      arcPtr->addOut(fileHash);
    }
}
