#ifndef __SPREAD_ARCRULE_HPP_
#define __SPREAD_ARCRULE_HPP_

#include "rule.hpp"
#include "dir/directory.hpp"
#include <assert.h>

namespace Spread
{
  // Archive rule
  struct ArcRule : Rule
  {
    DirectoryCPtr dir;

    ArcRule(const Hash &arc, DirectoryCPtr _dir,
            const std::string &rulestr)
      : Rule(RST_Archive, rulestr), dir(_dir)
    {
      addDep(arc);

      Hash::DirMap::const_iterator it;
      for(it = dir->dir.begin(); it != dir->dir.end(); it++)
        addOut(it->second);
    }

    /* Used for "blind" archives, ie. ones where we don't preload the
       directory up front, but rather use whatever is in the archive
       as our filename guide. Since we are working with hashed
       archives with known content, this is still 100% predictable and
       deterministic, and saves us the trouble of downloading the
       directory file separately.
    */
    ArcRule(const Hash &arc, const std::string &rulestr)
      : Rule(RST_Archive, rulestr)
    {
      addDep(arc);
    }

    // Get ArcRule pointer from a Rule pointer
    static const ArcRule *get(const Rule *r)
    {
      assert(r->type == RST_Archive);
      const ArcRule *ur = dynamic_cast<const ArcRule*>(r);
      assert(ur != NULL);
      return ur;
    }
  };
}
#endif
