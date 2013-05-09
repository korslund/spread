#include "askqueue.hpp"
#include <queue>
#include <job/thread.hpp>
#include <boost/thread/mutex.hpp>

#ifdef NEED_LOCKGUARD
#include <boost/thread/lock_guard.hpp>
#endif

#define LOCK boost::lock_guard<boost::mutex> lock(mutex)

using namespace Spread;

template <class T>
class SafeQueue
{
  std::queue<T> queue;
  boost::mutex mutex;

public:

  void push(const T& t)
  {
    LOCK;
    queue.push(t);
  }

  bool pop(T &t)
  {
    LOCK;
    if(queue.empty()) return false;

    t = queue.front();
    queue.pop();
    return true;
  }
};

struct AskQueue::_Internal
{
  SafeQueue<AskPtr> askList;
};

AskQueue::AskQueue() : ptr(new _Internal) {}

void AskQueue::push(AskPtr ask) { ptr->askList.push(ask); }

bool AskQueue::pushWait(AskPtr ask, JobInfoPtr info)
{
  push(ask);
  while(!ask->ready)
    {
      if(info)
        {
          if(ask->abort)
            info->abort();

          if(info->checkStatus()) return true;
        }
      else if(ask->abort)
        return true;

      Thread::sleep(0.05);
    }

  return false;
}

bool AskQueue::pop(AskPtr &ask)
{
  return ptr->askList.pop(ask);
}
