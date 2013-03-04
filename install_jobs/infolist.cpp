#include "tlist.hpp"

using namespace Spread;

void InfoList::remove(const Hash::DirMap &files)
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
bool InfoList::fetchOrInsert(const Hash &hash, JobInfoPtr &ptr)
{
  LockGuard lock(mutex);
  HMap::iterator it = list.find(hash);
  if(it == list.end())
    {
      assert(ptr);
      assert(!ptr->hasStarted());
      list[hash] = ptr;
      return false;
    }

  ptr = it->second;
  assert(ptr);
  return true;
}
