#include "spread.hpp"
#include "cache/cache.hpp"
#include "sr0/sr0.hpp"
#include "rules/ruleset.hpp"
#include "rules/rule_loader.hpp"
#include "install/installer.hpp"
#include "dir/directory.hpp"
#include "job/thread.hpp"
#include "misc/readjson.hpp"
#include "tasks/download.hpp"
#include "hash/hash_stream.hpp"
#include <mangle/stream/servers/file_stream.hpp>
#include <mangle/stream/clients/copy_stream.hpp>
#include <boost/filesystem.hpp>
#include <stdexcept>
#include <vector>
#include <boost/thread/recursive_mutex.hpp>

using namespace Spread;

JobInfoPtr SpreadLib::download(const std::string &url,
                               const std::string &dest,
                               bool async)
{
  DownloadTask *job = new DownloadTask(url, dest);
  return Thread::run(job, async);
}

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
typedef std::vector<std::string> StrVec;

struct Pack
{
  HashVec dirs, hints;
  StrVec paths;
  std::string version;
};

// TODO: Should move this to a separate file
struct PackList
{
  typedef std::map<std::string, Pack> PackMap;
  PackMap packs;

  static void copy(const Json::Value &from, HashVec &to, StrVec *paths = NULL)
  {
    using namespace std;

    if(from.isNull()) return;
    if(!from.isArray()) fail("Invalid pack file");

    to.resize(from.size());
    if(paths) paths->resize(from.size());

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

        to[i] = Hash(hash);

        if(paths) (*paths)[i] = path;
      }
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

        copy(v["dirs"], p.dirs, &p.paths);
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
  // since our last run or if it hasn't been loaded at all yet.
  void update(const std::string &channel)
  {
    // Does the channel exist?
    if(allPacks.find(channel) != allPacks.end())
      {
        // Yes. Check if it is currently being updated.
        JobInfoPtr &info = chanJobs[channel];

        // There has been no updates since our last load, so there's
        // no point in reloading.
        if(!info) return;

        if(info->isSuccess())
          {
            // The job is done, the data is usable.
            info.reset();
            assert(!chanJobs[channel]);
          }
        else
          /* The job is currently writing the data, meaning we can't
             safely load it right now.
           */
          return;
      }

    /* Load the data. Will throw on errors, which is what we expect.
     */
    try
      {
        loadRulesJsonFile(rules, chanPath(channel, "rules.json"));
        allPacks[channel].load(chanPath(channel, "packs.json"));
      }
    catch(std::exception &e)
      {
        fail("Error loading channel " + channel + ".\nDETAILS: " + e.what());
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
  { try { bf::remove_all(cache.tmpDir); } catch(...) {} }
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

/*
std::string SpreadLib::getPackHash(const std::string &channel,
                                   const std::string &package)
{
  LOCK;
  const Pack& p = ptr->getPack(channel,package);

  /* The idea here is to return the DirHash for the final output
     directory. This is done by "melding" all the output directories of
     the package into one (hints are irrelevant), and then hashing that.
  * /

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
*/

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
    inst->addDir(p.dirs[i], true, p.paths[i]);
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
  // Doesn't need LOCK, since CacheIndex does its own internal locking
  return ptr->cache.index.addFile(abs(file)).toString();
}

void SpreadLib::cacheCopy(const std::vector<std::string> &inputs,
                          const std::vector<std::string> &outputs,
                          JobInfoPtr info)
{
  assert(inputs.size() == outputs.size());

  int64_t totalSize = 0, cur = 0;

  // If we're keeping stats, add up the input file sizes
  if(info)
    {
      for(int i=0; i<inputs.size(); i++)
        totalSize += bf::file_size(inputs[i]);
      info->setProgress(0, totalSize);
    }

  // Output list of files and hashes
  Hash::DirMap dirmap;

  // Start copying
  using namespace Mangle::Stream;
  for(int i=0; i<inputs.size(); i++)
    {
      const std::string &from = inputs[i];
      const std::string &to = outputs[i];

      parent(to);
      HashStreamPtr out(new HashStream(to, true));
      CopyStream::copy(FileStream::Open(from), out);

      Hash hash = out->finish();
      cur += hash.size();

      if(info)
        info->setProgress(cur, totalSize);

      // Store both files in the output
      dirmap[from] = hash;
      dirmap[to] = hash;
    }

  // Add the entries to the cache index
  ptr->cache.index.addMany(dirmap);
}

std::string SpreadLib::cacheCopy(const std::string &from, const std::string &to)
{
  using namespace Mangle::Stream;
  parent(to);
  Hash res;
  {
    HashStreamPtr out(new HashStream(to, true));
    CopyStream::copy(FileStream::Open(from), out);
    res = out->finish();
  }
  ptr->cache.index.addFile(from, res);
  ptr->cache.index.addFile(to, res);
  return res.toString();
}
