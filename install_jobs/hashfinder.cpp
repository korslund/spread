#include "hashfinder.hpp"
#include <rules/urlrule.hpp>
#include <rules/arcrule.hpp>

using namespace Spread;

bool HashFinder::findHash(const Hash &hash, HashSource &out, const std::string &target)
{
  out.hash = hash;
  out.type = TST_None;
  out.value.clear();
  out.deps.clear();
  out.dir = NULL;

  // Check for existing files first
  int stat = Cache::CI_ElseWhere;

  if(target != "")
    stat = cache.getStatus(target, hash);

  if(stat == Cache::CI_Match)
    {
      // This file already exists.
      out.type = TST_InPlace;
      out.value = target;
      return true;
    }

  if(stat == Cache::CI_ElseWhere)
    {
      // File either exists elsewhere, or target == "". Get cache
      // location, if any.
      const std::string &existing = cache.findHash(hash);
      if(existing != "")
        {
          out.type = TST_File;
          out.value = existing;
          return true;
        }
    }

  // File does not exist in the file system. Check the rules.
  const Rule *r = rules.findRule(hash);
  if(!r) return false;

  // Copy dependency list
  out.deps = r->deps;

  if(r->type == RST_URL)
    {
      out.type = TST_Download;
      out.value = URLRule::get(r)->url;
      assert(out.deps.size() == 0);
    }
  else if(r->type == RST_Archive)
    {
      out.type = TST_Archive;
      out.dir = &ArcRule::get(r)->dir->dir;
      assert(out.deps.size() == 1);
    }
  else assert(0);

  return true;
}
