#include "spread.hpp"
#include "cache/cache.hpp"
#include "sr0/sr0.hpp"
#include "rules/ruleset.hpp"
#include "rules/rule_loader.hpp"
#include "install/installer.hpp"
#include "dir/directory.hpp"
#include "job/thread.hpp"
#include "misc/readjson.hpp"
#include <boost/filesystem.hpp>
#include <stdexcept>
#include <vector>
#include <boost/thread/recursive_mutex.hpp>

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

typedef std::vector<Hash> HashVec;

struct Pack
{
  HashVec dirs, hints;
  std::string version;
};

// TODO: Should move this to a separate file
struct PackList
{
  typedef std::map<std::string, Pack> PackMap;
  PackMap packs;

  void copy(const Json::Value &from, HashVec &to)
  {
    if(from.isNull()) return;
    if(!from.isArray()) fail("Invalid pack file");

    to.resize(from.size());

    for(int i=0; i<from.size(); i++)
      to[i] = Hash(from[i].asString());
  }

  void load(const std::string &file)
  {
    packs.clear();

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

  const Pack& get(const std::string &pack) const
  {
    PackMap::const_iterator it = packs.find(pack);
    if(it == packs.end())
      fail("Unknown package " + pack);
    return it->second;
  }
};

#define LOCK boost::lock_guard<boost::recursive_mutex> lock(ptr->mutex)

struct SpreadLib::_Internal
{
  Cache::Cache cache;
  RuleSet rules;
  boost::recursive_mutex mutex;

  // This keeps track of updates in progress for a given channel
  std::map<std::string, JobInfoPtr> chanJobs;
  std::map<std::string, bool> wasUpdated;
  std::map<std::string, PackList> allPacks;

  /* True if the given channel has a JobInfoPtr entry that has
     finished.

     This means that a new ruleset has been installed since our last
     run.
  */
  bool isUpdated(const std::string &channel)
  {
    JobInfoPtr &info = chanJobs[channel];
    bool res = info && info->isSuccess();
    if(res)
      {
        info.reset();
        assert(!chanJobs[channel]);
      }
    return res;
  }

  const Pack& getPack(const std::string &channel,
                      const std::string &pack)
  {
    update(channel);
    try { return allPacks[channel].get(pack); }
    catch(...)
      {
        fail("Unknown package " + channel + "/" + pack);
      }
  }

  // Load the newest ruleset and packlist, if it has been updated
  // since our last run
  void update(const std::string &channel)
  {
    if(isUpdated(channel))
      {
        loadRulesJsonFile(rules, chanPath(channel, "rules.json"));
        allPacks[channel].load(chanPath(channel, "packs.json"));
      }
  }

  JobInfoPtr &setUpdateInfo(const std::string &channel)
  {
    JobInfoPtr &res = chanJobs[channel];

    if(res && !res->isFinished())
      fail("Update already in progress for channel 'channel'");

    return res;
  }

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

void SpreadLib::setURLCallback(CBFunc cb) { ptr->rules.setURLCallback(cb); }

bool SpreadLib::wasUpdated(const std::string &channel) const
{ return ptr->wasUpdated[channel]; }

JobInfoPtr SpreadLib::updateFromURL(const std::string &channel,
                                    const std::string &url,
                                    bool async)
{
  LOCK;
  return ptr->setUpdateInfo(channel) =
    SR0::fetchURL(url, ptr->chanPath(channel), ptr->cache, async,
                  &ptr->wasUpdated[channel]);
}

JobInfoPtr SpreadLib::updateFromFile(const std::string &channel,
                                     const std::string &path,
                                     bool async)
{
  LOCK;
  return ptr->setUpdateInfo(channel) =
    SR0::fetchFile(path, ptr->chanPath(channel), ptr->cache, async,
                   &ptr->wasUpdated[channel]);
}

std::string SpreadLib::getPackVersion(const std::string &channel,
                                      const std::string &package)
{
  LOCK;
  return ptr->getPack(channel, package).version;
}

std::string SpreadLib::getPackHash(const std::string &channel,
                                   const std::string &package)
{
  LOCK;
  const Pack& p = ptr->getPack(channel,package);

  /* The idea here is to return the DirHash for the final output
     directory. This is done by "melding" all the output directories of
     the package into one (hints are irrelevant), and then hashing that.
  */

  // If there's less than two, this is easy
  if(p.dirs.size() == 0)
    return "00";
  if(p.dirs.size() == 1)
    return p.dirs[0].toString();

  // Otherwise, collect all the dirs into one
  Directory dir;
  for(int i=0; i<p.dirs.size(); i++)
    {
      const Hash &dh = p.dirs[i];
      DirectoryCPtr p = ptr->cache.loadDir(dh);
      if(!p) fail("Failed to load dir " + dh.toString() + " from package " +
                  channel + "/" + package);
      dir.add(*p);
    }

  // Return the collected dirhash
  return dir.hash().toString();
}

JobInfoPtr SpreadLib::install(const std::string &channel,
                              const std::string &package,
                              const std::string &where,
                              std::string *version,
                              bool async)
{
  LOCK;
  const Pack& p = ptr->getPack(channel,package);
  if(version) *version = p.version;

  Installer *inst = new Installer(ptr->cache, ptr->rules, abs(where));

  for(int i=0; i<p.dirs.size(); i++)
    inst->addDir(p.dirs[i]);
  for(int i=0; i<p.hints.size(); i++)
    inst->addHint(p.hints[i]);

  return Thread::run(inst, async);
}

JobInfoPtr SpreadLib::unpackURL(const std::string &url, const std::string &where,
                                bool async)
{
  LOCK;
  return SR0::fetchURL(url, abs(where), ptr->cache, async);
}

std::string SpreadLib::cacheFile(const std::string &file)
{
  LOCK;
  return ptr->cache.index.addFile(abs(file)).toString();
}
