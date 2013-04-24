#include "packlist.hpp"
#include <map>
#include <stdexcept>
#include <misc/readjson.hpp>
#include "infojson.hpp"

using namespace Spread;

typedef std::map<std::string,PackInfo*> Lookup;

struct PackList::_Internal
{
  PackInfoList list;
  Lookup lookup;
};

PackList::PackList() { ptr.reset(new _Internal); }

PackInfoList PackList::getList() const { return ptr->list; }

static void fail(const std::string &msg)
{ throw std::runtime_error(msg); }

void PackList::loadJson(const std::string &file, const std::string &channel)
{
  assert(file != "");
  clear();

  using namespace Json;

  Value root = ReadJson::readJson(file);

  if(!root.isObject()) fail("Invalid pack file " + file);

  Value::Members keys = root.getMemberNames();
  ptr->list.resize(keys.size());
  for(int i=0; i<keys.size(); i++)
    {
      const std::string &key = keys[i];
      PackInfo &p = ptr->list[i];
      ptr->lookup[key] = &p;
      jsonToInfo(p, root[key], channel, key);
    }
}

PackInfo PackList::get(const std::string &pack) const
{
  Lookup::const_iterator it = ptr->lookup.find(pack);
  if(it == ptr->lookup.end())
    fail("Unknown package " + pack);
  return *it->second;
}

void PackList::clear()
{
  ptr->list.clear();
  ptr->lookup.clear();
}
