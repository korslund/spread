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
