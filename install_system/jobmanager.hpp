#ifndef __SPREAD_JOBMANAGER_HPP_
#define __SPREAD_JOBMANAGER_HPP_

#include <install_dir/installer.hpp>
#include <parent_job/jobholder.hpp>
#include <parent_job/userask.hpp>
#include <rules/ruleset.hpp>
#include <cache/cache.hpp>
#include <misc/logger.hpp>

namespace Spread
{
  struct JobManager : JobHolder
  {
    JobManager(Cache::Cache &_cache);

    /* Get next error or question from the jobs. If the UserAsk::abort
       member is set, then this is an error message. If not, it is a
       question to the user which you may respond to (or choose to
       abort.)
     */
    StringAskPtr getNextError();

    /* Create an installer job that is set up to be used with this
       manager. After adding the files you want using inst->addDir()
       etc, start the job through addInst().

       Parameter 'destDir' specifies where to install files.
     */
    InstallerPtr createInstaller(const std::string &destDir, RuleSet &rules);
    JobInfoPtr addInst(InstallerPtr);

    /* Set log output.
     */
    void setLogger(const std::string &filename);
    void setLogger(std::ostream *strm);
    void setLogger(Misc::LogPtr logger, bool trd=true);
    void setPrintLogger();

    Cache::Cache &cache;

  private:
    struct _Internal;
    boost::shared_ptr<_Internal> ptr;

    void handleError(const std::string &msg);
  };

  typedef boost::shared_ptr<JobManager> JobManagerPtr;
}
#endif
