#ifndef __SPREAD_TARGET_HPP_
#define __SPREAD_TARGET_HPP_

#include <install_system/ihashfinder.hpp>
#include <parent_job/execjob.hpp>
#include "ijobmaker.hpp"

namespace Spread
{
  struct TargetOwner
  {
    virtual std::string fetchFile(const Hash &hash, JobPtr &job,
                                  const std::string &target="") = 0;
    virtual void brokenURL(const Hash &hash, const std::string &url) = 0;
    virtual void notifyFiles(const Hash::DirMap &files) = 0;
  };

  struct HashTask;
  struct Target : ExecJob
  {
    HashSource src;
    Hash::DirMap output;
    Target(TargetOwner *o, IJobMaker &m) : owner(o), maker(m) {}

  private:
    TargetOwner *owner;
    IJobMaker &maker;
    std::string fetchFile(const Hash &hash);
    bool execHashTask(HashTaskBase *htask, bool failOnError);
    void doJob();
  };

  typedef boost::shared_ptr<Target> TargetPtr;
}
#endif
