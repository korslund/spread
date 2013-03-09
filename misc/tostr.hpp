#ifndef __MISC_TOSTR_HPP_
#define __MISC_TOSTR_HPP_

#include <sstream>
#include <string>

template <typename X>
static std::string toStr(X i)
{
  std::stringstream str;
  str << i;
  return str.str();
}

#endif
