#include "arcruleset.hpp"
#include "arcrule.hpp"
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

ArcRuleSet::ArcRuleSet(RuleFinder *_base)
  : base(_base)
{
  ptr.reset(new _Internal);
}

void ArcRuleSet::findAllRules(const Hash &hash, RuleList &output) const
{
  const Rule *r = ptr->findFile(hash);
  if(r) output.insert(r);

  if(base)
    base->findAllRules(hash, output);
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

void ArcRuleSet::addArchive(const Hash arcHash, DirectoryCPtr dir,
                            const std::string &ruleString)
{
  ArcPtr arcPtr(new ArcRule(arcHash, dir, ruleString));

  /* Loop through all the output hashes, and add a lookup for each of
     them pointing to this archive.
  */
  for(int i=0; i<arcPtr->outputs.size(); i++)
    {
      const Hash &fileHash = arcPtr->outputs[i];
      ptr->files[fileHash] = arcPtr;
    }
}
