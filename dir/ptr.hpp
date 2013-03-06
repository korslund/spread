#ifndef __SPREAD_DIR_PTR_HPP_
#define __SPREAD_DIR_PTR_HPP_

#include <hash/hash.hpp>
#include <boost/shared_ptr.hpp>

namespace Spread
{
  typedef boost::shared_ptr<Hash::DirMap> DirPtr;
  typedef boost::shared_ptr<const Hash::DirMap> DirCPtr;
}

#endif
