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
      : message(msg), ready(false), abort(false) {}

    // This makes the class polymorphic, which means we can use
    // dynamic_cast<> on child classes.
    virtual ~UserAsk() {}
  };

  struct StringAsk;
  typedef boost::shared_ptr<UserAsk> AskPtr;
  typedef boost::shared_ptr<StringAsk> StringAskPtr;

  struct StringAsk : UserAsk
  {
    std::vector<std::string> options;
    int selection;

    void addOption(const std::string &opt)
    { options.push_back(opt); }

    void select(int i)
    {
      assert(i >= 0 && i < options.size());
      selection = i;
      ready = true;
    }

    StringAsk(const std::string &question)
      : UserAsk(question), selection(-1) {}

    static StringAskPtr cast(AskPtr ask)
    { return boost::dynamic_pointer_cast<StringAsk>(ask); }
  };
}
#endif
