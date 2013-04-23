#include "infojson.hpp"
#include <misc/readjson.hpp>
#include <stdexcept>

using namespace Spread;

static void toDirs(const Json::Value &from, StrVec &dirs, StrVec &paths)
{
  using namespace std;

  if(from.isNull()) return;
  if(!from.isArray()) 
    throw std::runtime_error("Error parsing pack file");

  dirs.resize(from.size());
  paths.resize(from.size());

  for(int i=0; i<from.size(); i++)
    {
      // Get the input string from the hash list
      string hash = from[i].asString();
      string path;

      // Split out the path part, if any
      size_t pos = hash.find(' ');
      if(pos != string::npos)
        {
          path = hash.substr(pos+1);
          hash = hash.substr(0,pos);
        }

      dirs[i] = hash;
      paths[i] = path;
    }
}

static Json::Value fromDirs(const StrVec &dirs, const StrVec &paths)
{
  assert(dirs.size() == paths.size());

  Json::Value res;
  for(int i=0; i<dirs.size(); i++)
    {
      std::string str = dirs[i];
      if(paths[i] != "") str += " " + paths[i];
      res.append(str);
    }
  return res;
}

Json::Value Spread::infoToJson(const PackInfo &info)
{
  Json::Value res;
  if(info.version != "")
    res["version"] = info.version;
  res["install_size"] = info.installSize;
  res["dirs"] = fromDirs(info.dirs, info.paths);
  return res;
}

void Spread::jsonToInfo(PackInfo &info, const Json::Value &val,
                        const std::string &channel, const std::string &package)
{
  toDirs(val["dirs"], info.dirs, info.paths);
  info.channel = channel;
  info.package = package;
  info.version = val["version"].asString();
  info.installSize = val["install_size"].asUInt();
}
