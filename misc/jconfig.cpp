#include "jconfig.hpp"
#include "readjson.hpp"
#include "comp85.hpp"
#include <stdexcept>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/filesystem.hpp>

using namespace Misc;

struct JConfig::_JConfig_Hidden
{
  Json::Value val; 
  boost::recursive_mutex mutex;

  void L() { mutex.lock(); }
  void U() { mutex.unlock(); }

  Json::Value LG(const std::string &name, const Json::Value &v)
  {
    L();
    Json::Value res = val.get(name, v);
    U();
    return res;
  }
};

JConfig::JConfig(const std::string &_file)
  : file(_file)
{
  p = new _JConfig_Hidden;
  load();
}
JConfig::~JConfig() { delete p; }

/* Mutex notes:

   The mutex is intended primarily to protect the setter and getter
   functions to avoid multiple simultaneous access to the Json::Value
   or the config file.
 */

void JConfig::load()
{
  // Never fail. A missing or invalid file is OK.
  if(file == "") return;

  p->L();
  try { p->val = ReadJson::readJson(file); }
  catch(...) {}
  p->U();
}

void JConfig::save()
{
  namespace bs = boost::filesystem;

  if(file == "")
    return;

  p->L();
  if(bs::exists(file))
    {
      std::string old = file + ".old";
      if(bs::exists(old))
        bs::remove(old);
      bs::rename(file,old);
    }
  ReadJson::writeJson(file, p->val);
  p->U();
}

void JConfig::setBool(const std::string &name, bool b)
{ p->L(); p->val[name] = b; save(); p->U(); }

void JConfig::setInt(const std::string &name, int i)
{ p->L(); p->val[name] = i; save(); p->U(); }

void JConfig::set(const std::string &name, const std::string &value)
{ p->L(); p->val[name] = value; save(); p->U(); }

bool JConfig::getBool(const std::string &name, bool def)
{ return p->LG(name,def).asBool(); }

int JConfig::getInt(const std::string &name, int def)
{ return p->LG(name,def).asInt(); }

void JConfig::setInt64(const std::string &name, int64_t i)
{ setData(name, &i, 8); }

int64_t JConfig::getInt64(const std::string &name, int64_t def)
{
  int64_t res;
  try { getData(name, &res, 8); }
  catch(...) { res=def; }
  return res;
}

bool JConfig::remove(const std::string &name)
{
  p->L();
  // removeMember() returns a Null value if nothing was removed.
  bool res = !p->val.removeMember(name).isNull();
  save();
  p->U();
}

std::string JConfig::get(const std::string &name, const std::string &def)
{ return p->LG(name,def).asString(); }

bool JConfig::has(const std::string &name)
{ return p->val.isMember(name); }

std::vector<std::string> JConfig::getNames()
{ return p->val.getMemberNames(); }

void JConfig::setData(const std::string &name, const void *p, size_t num)
{
  set(name, Comp85::encode(p,num));
}

void JConfig::getData(const std::string &name, void *p, size_t num)
{
  std::string val = get(name);
  if(Comp85::calc_binary(val.size()) != num)
    throw std::runtime_error("Space mismatch in field '" + name + "'");
  Comp85::decode(get(name), p);
}

