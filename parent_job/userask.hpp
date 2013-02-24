#ifndef __SPREAD_USERASK_HPP_
#define __SPREAD_USERASK_HPP_

#include <string>
#include <vector>
#include <assert.h>

namespace Spread
{
  struct UserAsk
  {
    std::string message;
    bool ready, abort;

    void abortJob() { abort = true; }

    UserAsk(const std::string &msg)
      : ready(false), abort(false) {}
    virtual ~UserAsk() {}
  };

  struct StringAsk : UserAsk
  {
    std::vector<std::string> options;
    int selection;

    void select(int i)
    {
      assert(i >= 0 && i < options.size());
      selection = i;
      ready = true;
    }

    StringAsk() : selection(-1) {}

    static StringAsk* handle(UserAsk *ask) { return dynamic_cast<StringAsk*>(ask); }
  };
}
#endif
