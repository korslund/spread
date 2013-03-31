#include "spread.hpp"
#include "sr0/sr0.hpp"
#include "rules/ruleset.hpp"
#include "rules/rule_loader.hpp"
#include "tasks/download.hpp"
#include "hash/hash_stream.hpp"
#include <mangle/stream/servers/file_stream.hpp>
#include <mangle/stream/clients/copy_stream.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <job/thread.hpp>

//#define PRINT_DEBUG
#ifdef PRINT_DEBUG
#include <iostream>
#define PRINT(a) std::cout << a << "\n"
#else
#define PRINT(a)
#endif

using namespace Spread;
namespace bf = boost::filesystem;

JobInfoPtr SpreadLib::download(const std::string &url,
                               const std::string &dest,
                               bool async)
{
  DownloadTask *job = new DownloadTask(url, dest);
  return Thread::run(job, async);
}

static std::string abs(const bf::path &path)
{
  return bf::absolute(path).string();
}

static void parent(const bf::path &file)
{
  bf::create_directories(file.parent_path());
}

struct SpreadLib::_Internal
{
  Cache::Cache cache;
  RuleSet rules;
  boost::recursive_mutex mutex;
  JobManagerPtr manager;

  // This keeps track of updates in progress for a given channel
  bf::path repoDir;

  std::map<std::string, bool> wasUpdated;

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
  {
    manager->finish();
    manager->getInfo()->abort();
    try { bf::remove_all(cache.tmpDir); } catch(...) {}
  }
};

#define LOCK boost::lock_guard<boost::recursive_mutex> lock(ptr->mutex)

SpreadLib::SpreadLib(const std::string &outDir, const std::string &tmpDir)
{
  PRINT("SpreadLib()");
  ptr.reset(new _Internal);

  ptr->repoDir = abs(outDir);
  PRINT("  repoDir=" << ptr->repoDir);

  ptr->cache.index.load(ptr->getPath("cache.conf"));
  ptr->cache.tmpDir = abs(tmpDir);
  ptr->cache.files.basedir = ptr->getPath("cache/");
  ptr->manager.reset(new JobManager(ptr->cache));

  PRINT("  Starting JobManager");
  Thread::run(ptr->manager);
}

JobManagerPtr SpreadLib::getJobManager() const { return ptr->manager; }

void SpreadLib::setURLCallback(CBFunc cb) { ptr->rules.setURLCallback(cb); }

bool SpreadLib::wasUpdated(const std::string &channel) const
{ return ptr->wasUpdated[channel]; }

JobInfoPtr SpreadLib::updateFromURL(const std::string &channel,
                                    const std::string &url,
                                    bool async)
{
  LOCK;
  // Load any existing data into memory, ignore errors.
  try { ptr->chan.load(channel); } catch(...) {}

  return ptr->setUpdateInfo(channel) =
    SR0::fetchURL(url, ptr->chanPath(channel), ptr->manager, async,
                  &ptr->wasUpdated[channel]);
}

JobInfoPtr SpreadLib::updateFromFile(const std::string &channel,
                                     const std::string &path,
                                     bool async)
{
  LOCK;
  try { ptr->chan.load(channel); } catch(...) {}
  return ptr->setUpdateInfo(channel) =
    SR0::fetchFile(path, ptr->chanPath(channel), ptr->manager, async,
                   &ptr->wasUpdated[channel]);
}

const SpreadLib::PackInfo &SpreadLib::getPackInfo(const std::string &channel,
                                                  const std::string &package) const
{
  LOCK;
  return *ptr->getPack(channel,package).info;
}

const SpreadLib::InfoList &SpreadLib::getInfoList(const std::string &channel) const
{
  LOCK;
  return ptr->getList(channel).infoList;
}

JobInfoPtr SpreadLib::installPack(const std::string &channel,
                                  const std::string &package,
                                  const std::string &where,
                                  std::string *version,
                                  bool async,
                                  bool doUpgrade,
                                  bool enableAsk)
{
  assert(channel != "" && package != "" && where != "");
  LOCK;
  
  const PackInfo& p = ptr->getPack(channel,package);
  if(version) *version = p.info->version;

  InstallerPtr inst = ptr->manager->createInstaller(abs(where), ptr->rules);

  for(int i=0; i<p.dirs.size(); i++)
    inst->addDir(p.dirs[i], p.paths[i]);

  if(doUpgrade)
    {
      const PackStatus *ps = getPackStatus(channel, package, where);
    }

  JobInfoPtr info = ptr->manager->addInst(inst);

  // TODO: Add monitor job to update our status

  return info;
}

JobInfoPtr SpreadLib::uninstallPack(const std::string &channel,
                                    const std::string &package,
                                    const std::string &where,
                                    bool async)
{
  blah; assert(0); // TODO: Not updated yet
  LOCK;
}

const SpreadLib::PackStatus *SpreadLib::getPackStatus(const std::string &channel,
                                                      const std::string &package,
                                                      const std::string &where) const
{
  assert(channel != "" && package != "");
  StatusList lst;

  getStatusList(lst, channel, package, where);

  // Return first element in the list, if any
  if(lst.size()) return *lst.begin();

  // No element found
  return NULL;
}

void SpreadLib::getStatusList(SpreadLib::StatusList &output,
                              const std::string &channel,
                              const std::string &package,
                              const std::string &where) const
{
  LOCK;

  // TODO: Ensure list is updated first

  RealStatusList::const_iterator it;
  const RealStatusList &list = ptr->blah;
  for(it = list.begin(); it != list.end(); it++)
    {
      const PackStatus &s = it->stat;
      if(channel != "" && channel != s.pack->channel) continue;
      if(package != "" && package != s.pack->package) continue;

      // TODO: We should do a full exists,is_directory,equivalence
      // check here
      if(where != "" && abs(where) != s.pack->path) continue;

      output.insert(&s);
    }
}

JobInfoPtr SpreadLib::unpackURL(const std::string &url, const std::string &where,
                                bool async)
{
  LOCK;
  return SR0::fetchURL(url, abs(where), ptr->manager, async);
}

void SpreadLib::verifyCache()
{
  PRINT("verifyCache() start");
  ptr->cache.index.verify();
  PRINT("verifyCache() done");
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

      // Store both files in the output
      dirmap[from] = hash;
      dirmap[to] = hash;

      if(info)
        {
          // Update progress, and also check for user aborts
          info->setProgress(cur, totalSize);
          if(info->checkStatus())
            break;
        }
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
