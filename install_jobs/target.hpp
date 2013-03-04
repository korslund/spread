#ifndef __SPREAD_TARGET_HPP_
#define __SPREAD_TARGET_HPP_

#include <install_system/ihashfinder.hpp>
#include <parent_job/execjob.hpp>
#include "ijobmaker.hpp"

namespace Spread
{
  struct Target : ExecJob
  {
    HashSource src;
    Hash::DirMap output;
    Target(IJobMaker &m) : maker(m) {}

    virtual std::string fetchFile(const Hash &hash) = 0;
    virtual void brokenURL(const Hash &hash, const std::string &url) = 0;
    virtual void notifyFiles(const Hash::DirMap &files) = 0;

  private:
    IJobMaker &maker;
    bool execHashTask(HashTaskBase *htask, bool failOnError);
    void doJob();
  };
}
#endif
