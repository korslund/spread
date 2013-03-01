#ifndef __SPREAD_MOVABLELOCK_HPP
#define __SPREAD_MOVABLELOCK_HPP

#include <boost/thread/recursive_mutex.hpp>
#include <boost/smart_ptr.hpp>

namespace Spread
{
  typedef boost::recursive_mutex Mutex;
  typedef boost::lock_guard<Mutex> LockGuard;
  typedef boost::shared_ptr<LockGuard> MovableLock;
}

#endif
