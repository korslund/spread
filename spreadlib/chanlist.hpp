#ifndef __SPREADLIB_CHANLIST_HPP_
#define __SPREADLIB_CHANLIST_HPP_

#include "packinfo.hpp"
#include <boost/shared_ptr.hpp>

namespace Spread
{
  struct ChanList
  {
    ChanList(const std::string &statFile);

  private:
    struct _Internal;
    boost::shared_ptr<_Internal> ptr;
  };
};

#endif
