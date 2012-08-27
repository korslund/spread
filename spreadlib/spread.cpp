#include "spread.hpp"
#include "cache/cache.hpp"
#include "sr0/sr0.hpp"
#include "rules/ruleset.hpp"
#include "rules/rule_loader.hpp"
#include "install/installer.hpp"
#include "job/thread.hpp"
#include "misc/readjson.hpp"
#include <boost/filesystem.hpp>
#include <stdexcept>
#include <vector>

using namespace Spread;

namespace bf = boost::filesystem;

static std::string abs(const bf::path &path)
{
  return absolute(path).string();
}

static void parent(const bf::path &file)
{
  bf::create_directories(file.parent_path());
}

static void fail(const std::string &msg)
{ throw std::runtime_error(msg); }

struct SpreadLib::_Internal
{
  Cache::Cache cache;

  bf::path repoDir;

  std::string getPath(const bf::path &file)
  {
    bf::path res = repoDir/file;
    parent(res);
    return res.string();
  }

  std::string chanPath(const std::string &channel, const std::string &file = "")
  {
    if(channel.find('/') != std::string::npos ||
       channel.find('\\') != std::string::npos)
      fail("Invalid channel name " + channel + " (contains slashes)");

    return getPath("channels/" + channel + "/" + file);
  }

  ~_Internal()
  { bf::remove_all(cache.tmpDir); }
};

SpreadLib::SpreadLib(const std::string &outDir, const std::string &tmpDir)
{
  ptr.reset(new _Internal);

  ptr->repoDir = abs(outDir);

  ptr->cache.index.load(ptr->getPath("cache.conf"));
  ptr->cache.tmpDir = abs(tmpDir);
}

JobInfoPtr SpreadLib::updateFromURL(const std::string &channel,
                                    const std::string &url,
                                    bool async)
{
  return SR0::fetchURL(url, ptr->chanPath(channel), ptr->cache, async);
}

JobInfoPtr SpreadLib::updateFromFile(const std::string &channel,
                                     const std::string &path,
                                     bool async)
{
  return SR0::fetchFile(path, ptr->chanPath(channel), ptr->cache, async);
}

typedef std::vector<Hash> HashVec;

struct Pack
{
  HashVec dirs, hints;
  std::string version;
};

// TODO: Should move this to a separate file
struct PackList
{
  std::map<std::string, Pack> packs;

  void copy(const Json::Value &from, HashVec &to)
  {
    if(from.isNull()) return;
    if(!from.isArray()) fail("Invalid pack file");

    to.resize(from.size());

    for(int i=0; i<from.size(); i++)
      to[i] = Hash(from[i].asString());
  }

  PackList(const std::string &file)
  {
    using namespace Json;

    Value root = ReadJson::readJson(file);

    if(!root.isObject()) fail("Invalid pack file " + file);

    Value::Members keys = root.getMemberNames();
    for(int i=0; i<keys.size(); i++)
      {
        const std::string &key = keys[i];

        Pack &p = packs[key];
        Value v = root[key];

        copy(v["dirs"], p.dirs);
        copy(v["hints"], p.hints);
        p.version = v["version"].asString();
      }
  }

  const Pack& get(const std::string &pack)
  { return packs[pack]; }
};

JobInfoPtr SpreadLib::install(const std::string &channel,
                              const std::string &package,
                              const std::string &where,
                              std::string *version,
                              bool async)
{
  /* We assume installs are pretty rare, so for now just do it the
     hard way and reload the data every time. We can optimize this
     later.
  */
  RuleSet rules;
  loadRulesJsonFile(rules, ptr->chanPath(channel, "rules.json"));

  PackList packs(ptr->chanPath(channel, "packs.json"));
  const Pack& p = packs.get(package);

  if(version) *version = p.version;

  Installer *inst = new Installer(ptr->cache, rules, abs(where));

  for(int i=0; i<p.dirs.size(); i++)
    inst->addDir(p.dirs[i]);
  for(int i=0; i<p.hints.size(); i++)
    inst->addHint(p.hints[i]);

  JobInfoPtr inf = Thread::run(inst, async);
  return inf;

  //return Thread::run(inst, async);
}

JobInfoPtr SpreadLib::unpackURL(const std::string &url, const std::string &where,
                                bool async)
{
  return SR0::fetchURL(url, abs(where), ptr->cache, async);
}

std::string SpreadLib::cacheFile(const std::string &file)
{
  return ptr->cache.index.addFile(abs(file)).toString();
}
