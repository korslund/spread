#include "jconfig.hpp"
#include "readjson.hpp"
#include "comp85.hpp"
#include <stdexcept>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/filesystem.hpp>

using namespace Misc;

//#define PRINT_DEBUG
#ifdef PRINT_DEBUG
#include <iostream>
#define PRINT(a) std::cout << a << "\n"
#else
#define PRINT(a)
#endif

namespace bs = boost::filesystem;

static void parent(const bs::path &file)
{
  bs::create_directories(file.parent_path());
}

struct JConfig::_JConfig_Hidden
{
  Json::Value val; 
  boost::recursive_mutex mutex;

  Json::Value LG(const std::string &name, const Json::Value &v)
  {
    mutex.lock();
    Json::Value res = val.get(name, v);
    mutex.unlock();
    return res;
  }
};

#define LOCK boost::lock_guard<boost::recursive_mutex> lock(p->mutex)
#define WLOCK assert(!readOnly);LOCK

JConfig::JConfig(const std::string &_file, bool _readOnly)
  : readOnly(_readOnly), file(_file)
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
  // We never fail. A missing or invalid file is OK.
  if(file == "") return;

  LOCK;
  try
    {
      std::string toLoad = file;

      // If the file does not exist, but .old does, revert to the
      // transactional backup.
      if(!bs::exists(toLoad))
        {
          toLoad += ".old";
          if(!bs::exists(toLoad))
            return;
        }
      p->val = ReadJson::readJson(toLoad);
    }
  catch(...) {}
}

void JConfig::save()
{
  if(file == "")
    return;

  std::string newFile = file + ".new";
  std::string oldFile = file + ".old";

  WLOCK;

  // First, write to .new
  parent(newFile);
  ReadJson::writeJson(newFile, p->val);

  // Then rename existing file (if any) to .old
  if(bs::exists(file))
    {
      if(bs::exists(oldFile))
        bs::remove(oldFile);
      bs::rename(file,oldFile);
    }

  // Finally, move .new into place
  bs::rename(newFile, file);
}

void JConfig::setBool(const std::string &name, bool b)
{ WLOCK; p->val[name] = b; save(); }

void JConfig::setInt(const std::string &name, int i)
{ WLOCK; p->val[name] = i; save(); }

void JConfig::set(const std::string &name, const std::string &value)
{
  PRINT("Conf: Setting " << name << " => " << value);
  WLOCK;
  p->val[name] = value;
  save();
}

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
  WLOCK;
  // removeMember() returns a Null value if nothing was removed.
  bool res = !p->val.removeMember(name).isNull();
  save();
}

void JConfig::setMany(const std::map<std::string,std::string> &entries)
{
  if(entries.size() == 0) return;

  WLOCK;
  std::map<std::string,std::string>::const_iterator it;
  for(it = entries.begin(); it != entries.end(); it++)
    p->val[it->first] = it->second;
  save();
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

