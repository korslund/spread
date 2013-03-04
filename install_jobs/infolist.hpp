#ifndef __SPREAD_TARGET_LIST_HPP
#define __SPREAD_TARGET_LIST_HPP

#include <hash/hash.hpp>
#include <job/jobinfo.hpp>
#include "movablelock.hpp"

namespace Spread
{
  struct InfoList
  {
    MovableLock lock() { return MovableLock(new LockGuard(mutex)); }

    void remove(const Hash::DirMap &files);

    // Inserts element if missing. Returns true if found, false if
    // inserted.
    bool fetchOrInsert(const Hash &hash, JobInfoPtr &ptr);

  private:
    typedef std::map<Hash, JobInfoPtr> HMap;
    HMap list;
    Mutex mutex;
  };
}

#endif
