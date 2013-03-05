#ifndef __SPREAD_ASK_QUEUE_HPP_
#define __SPREAD_ASK_QUEUE_HPP_

#include "userask.hpp"
#include <job/jobinfo.hpp>

namespace Spread
{
  struct AskQueue
  {
    /* Push as new Ask request onto the queue.
     */
    void push(AskPtr ask);

    /* Push a request and wait for it to be handled. Returns true if
       the caller should abort.

       If you are running from a Job, it is HIGHLY recommended to pass
       along the JobInfoPtr so that the wait loop can respond properly
       to abort requests.
     */
    bool pushWait(AskPtr &ask, JobInfoPtr info = JobInfoPtr());

    /* Pop a value off the queue, or return false if there are no
       values.
     */
    bool pop(AskPtr &ask);

    AskQueue();

  private:
    struct _Internal;
    boost::shared_ptr<_Internal> ptr;
  };
}
#endif
