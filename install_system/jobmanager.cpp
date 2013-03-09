#include "jobmanager.hpp"

#include <boost/thread/recursive_mutex.hpp>
#include <install_jobs/leaffactory.hpp>
#include <install_dir/dir_install.hpp>
#include <parent_job/askqueue.hpp>
#include <boost/filesystem.hpp>
#include <misc/logger.hpp>
#include <dir/binary.hpp>
#include <job/thread.hpp>
#include <stdexcept>

using namespace Spread;

struct JobManager::_Internal : DirOwner
{
  AskQueue askQueue;
  Cache::Cache &cache;
  LeafFactory fact;
  Misc::LogPtr logPtr;

  _Internal(Cache::Cache &c) : cache(c) {}

  bool askWait(AskPtr ask, JobInfoPtr info) { return askQueue.pushWait(ask, info); }
  std::string getTmpName(const Hash &hash) { return cache.createTmpFilename(hash); }

  void loadDir(const std::string &file, Hash::DirMap &output,
               const Hash &check = Hash())
  {
    Dir::read(output, file);
    storeDir(output, check);
  }

  void storeDir(const Hash::DirMap &dir, const Hash &check = Hash())
  {
    Hash hash = Dir::hash(dir);
    if(check.isSet() && hash != check)
      throw std::runtime_error("storeDir(): mismatched dir hash (wanted " +
                               check.toString() + ", got " + hash.toString() + ")");
    /* This will delete the existing file if it's wrong, or return ""
       if the file is already in place */
    std::string file = cache.files.storePath(hash);
    if(file != "")
      {
        Dir::write(dir, file);
        cache.index.addFile(file, hash);
      }
  }

  void deleteFile(const std::string &path)
  {
    namespace bf = boost::filesystem;
    log("deleteFile(" + path + ")");
    try { bf::remove(path); } catch(...)
      { log("  delete failed, ignoring."); }
    cache.index.removeFile(path);
  }

  void moveFile(const std::string &from, const std::string &to)
  {
    namespace bf = boost::filesystem;
    log("moveFile("+from+" => "+to+")");
    bool didCopy = false;
    bf::create_directories(bf::path(to).parent_path());
    try { bf::rename(from, to); }
    catch(...)
      {
        log("  - move failed, fallback to copy");
        didCopy = true;
        bf::copy_file(from, to);
      }

    // Cache new before delete old - this is the least destructive if
    // addFile() throws.
    cache.index.addFile(to);

    if(didCopy)
      deleteFile(from);
    else
      cache.index.removeFile(from);
  }

  TreePtr copyTarget(const std::string &from)
  { return fact.copyTarget(*this, from); }
  TreePtr downloadTarget(const std::string &url)
  { return fact.downloadTarget(*this, url); }
  TreePtr unpackTarget(const Hash &dir)
  { return fact.unpackTarget(*this, dir); }
  TreePtr unpackBlindTarget(const std::string &where, const Hash &dir = Hash())
  { return fact.unpackBlindTarget(*this, where, dir); }

  void log(const std::string &msg)
  {
    if(logPtr) logPtr->log("trd="+Thread::getId() + ": " + msg);
  }

  typedef boost::recursive_mutex Mutex;
  typedef boost::lock_guard<Mutex> LockGuard;

  Mutex mutex;
  std::map<Hash, JobInfoPtr> running;

  Lock lock() { return Lock(new LockGuard(mutex)); }
  void notifyFiles(const Hash::DirMap &files)
  {
    LockGuard l(mutex);
    Hash::DirMap::const_iterator it;
    for(it = files.begin(); it != files.end(); it++)
      running[it->second] = JobInfoPtr();
  }

  JobInfoPtr getRunningTarget(const Hash &hash)
  {
    LockGuard l(mutex);
    return running[hash];
  }

  void setRunningTarget(const Hash &hash, JobInfoPtr ptr)
  {
    LockGuard l(mutex);
    assert(ptr);
    running[hash] = ptr;
  }
};

JobManager::JobManager(Cache::Cache &_cache, RuleSet &_rules)
  : rules(_rules)
{
  ptr.reset(new _Internal(_cache));
}

StringAskPtr JobManager::getNextError()
{
  AskPtr p;
  if(!ptr->askQueue.pop(p)) return StringAskPtr();

  StringAskPtr p2 = StringAsk::cast(p);
  assert(p2);
  return p2;
}

void JobManager::handleError(const std::string &msg)
{
  AskPtr ask(new StringAsk(msg));
  ask->abort = true;
  ptr->askQueue.push(ask);
}

InstallerPtr JobManager::createInstaller(const std::string &destDir)
{
  return InstallerPtr(new DirInstaller(*ptr, rules, ptr->cache.index, destDir));
}

void JobManager::addInst(InstallerPtr p)
{
  JobPtr job = boost::dynamic_pointer_cast<DirInstaller>(p);
  assert(job);
  add(job);
}

void JobManager::setLogger(const std::string &filename)
{
  ptr->logPtr.reset(new Misc::Logger(filename));
}

void JobManager::setLogger(std::ostream *strm)
{
  ptr->logPtr.reset(new Misc::Logger(strm));
}
