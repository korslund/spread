#ifndef __SPREAD_TARGET_LIST_HPP
#define __SPREAD_TARGET_LIST_HPP

#include <hash/hash.hpp>
#include <job/job.hpp>
#include "movablelock.hpp"

namespace Spread
{
  struct TargetList
  {
    MovableLock lock() { return MovableLock(new LockGuard(mutex)); }

    void remove(const Hash::DirMap &files);

    // Inserts element if missing. Returns true if found, false if
    // inserted.
    bool fetchOrInsert(const Hash &hash, JobPtr &ptr);

  private:
    typedef std::map<Hash, JobPtr> HMap;
    HMap list;
    Mutex mutex;
  };
}

#endif
