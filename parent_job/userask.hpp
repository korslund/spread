#ifndef __SPREAD_USERASK_HPP_
#define __SPREAD_USERASK_HPP_

#include <string>
#include <vector>
#include <assert.h>
#include <boost/smart_ptr.hpp>

namespace Spread
{
  struct UserAsk
  {
    std::string message;
    bool ready, abort;

    void abortJob() { abort = true; }

    UserAsk(const std::string &msg)
      : ready(false), abort(false) {}

    // This makes the class polymorphic, which means we can use
    // dynamic_cast<> on child classes.
    virtual ~UserAsk() {}
  };

  typedef boost::shared_ptr<UserAsk> AskPtr;

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

    static StringAsk* handle(AskPtr ask) { return dynamic_cast<StringAsk*>(ask.get()); }
  };
}
#endif
