#ifndef __SPREAD_LIB_INFOJSON_HPP_
#define __SPREAD_LIB_INFOJSON_HPP_

#include "packinfo.hpp"
#include <json/json.h>

namespace Spread
{
  extern Json::Value infoToJson(const PackInfo &info);
  extern void jsonToInfo(PackInfo &info, const Json::Value &val,
                         const std::string &channel,
                         const std::string &package);
}

#endif
