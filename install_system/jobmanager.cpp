#include "jobmanager.hpp"

// Ok, so I like ordering includes by line length ;-)
#include <boost/thread/recursive_mutex.hpp>
#include <install_jobs/leaffactory.hpp>
#include <install_dir/dir_install.hpp>
#include <parent_job/askqueue.hpp>
#include <boost/filesystem.hpp>
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
  bool logTrd;

  _Internal(Cache::Cache &c) : cache(c), logTrd(true) {}

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
  }

  void moveFile(const std::string &from, const std::string &to)
  {
    namespace bf = boost::filesystem;
    log("moveFile("+from+" => "+to+")");
    bf::create_directories(bf::path(to).parent_path());
    if(bf::exists(to)) bf::remove(to);
    try { bf::rename(from, to); }
    catch(...)
      {
        log("  - move failed, fallback to copy");
        bf::copy_file(from, to);
        deleteFile(from);
      }
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
    if(logPtr)
      {
        std::string out;
        if(logTrd) out = "trd="+Thread::getId() + ": ";
        out += msg;
        logPtr->log(out);
      }
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

JobManager::JobManager(Cache::Cache &_cache)
  : cache(_cache)
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

InstallerPtr JobManager::createInstaller(const std::string &destDir, RuleSet &rules, bool doAsk)
{
  return InstallerPtr(new DirInstaller(*ptr, rules, cache.index, destDir, doAsk));
}

JobInfoPtr JobManager::addInst(InstallerPtr p)
{
  JobPtr job = boost::dynamic_pointer_cast<DirInstaller>(p);
  assert(job);
  add(job);
  return job->getInfo();
}

void JobManager::setLogger(const std::string &filename)
{
  ptr->logPtr.reset(new Misc::Logger(filename));
}

void JobManager::setLogger(std::ostream *strm)
{
  ptr->logPtr.reset(new Misc::Logger(strm));
}

void JobManager::setLogger(Misc::LogPtr logger, bool trd)
{
  ptr->logPtr = logger;
  ptr->logTrd = trd;
}

void JobManager::setPrintLogger()
{
  setLogger(Misc::LogPtr(new Misc::Logger), false);
}
