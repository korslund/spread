#include "tools.hpp"

using namespace Spread;

typedef Hash::DirMap DirMap;

void Spread::Dir::add(DirMap &dir, const DirMap &d, const std::string &prefix)
{
  for(DirMap::const_iterator it = d.begin(); it != d.end(); it++)
    dir[prefix+it->first] = it->second;
}
