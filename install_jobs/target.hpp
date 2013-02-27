#ifndef __SPREAD_TARGET_HPP_
#define __SPREAD_TARGET_HPP_

#include <install_system/ihashfinder.hpp>
#include <parent_job/execjob.hpp>

namespace Spread
{
  struct TargetOwner
  {
    virtual std::string fetchTmpFile(const Hash &hash, JobPtr &job) = 0;
    virtual void brokenURL(const Hash &hash, const std::string &url) = 0;
    virtual void addToCache(const Hash::DirMap &files) = 0;
  };

  struct HashTask;
  struct Target : ExecJob
  {
    HashSource src;
    Hash::DirMap output;

    void setMaster(JobInfoPtr inf)
    {
      src.type = TST_None;
      info = inf;
    }

    Target(TargetOwner *o) : owner(o) {}

  private:
    TargetOwner *owner;

    std::string fetchFile(const Hash &hash);
    bool execHashTask(HashTask *htask, bool failOnError);
    void doJob();
  };
}
#endif
