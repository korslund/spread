#include "tlist.hpp"

using namespace Spread;

void TargetList::remove(const Hash::DirMap &files)
{
  LockGuard lock(mutex);
  Hash::DirMap::const_iterator it;
  for(it = files.begin(); it != files.end(); it++)
    {
      HMap::iterator it2 = list.find(it->second);
      if(it2 != list.end())
        {
          assert(it2->second);
          list.erase(it2);
        }
    }
}

// Inserts element if missing. Returns true if found, false if
// inserted.
bool TargetList::fetchOrInsert(const Hash &hash, JobPtr &ptr)
{
  LockGuard lock(mutex);
  HMap::iterator it = list.find(hash);
  if(it == list.end())
    {
      assert(ptr);
      assert(!ptr->getInfo()->hasStarted());
      list[hash] = ptr;
      return false;
    }

  ptr = it->second;
  assert(ptr);
  return true;
}
