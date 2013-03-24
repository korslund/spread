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

    /* Get next error or question from the jobs. This is used for
       questions from the system, which you must either respond to or
       abort. Returns an empty ptr if when there are no more
       questions.
     */
    StringAskPtr getNextError();

    /* Create an installer job that is set up to be used with this
       manager. After adding the files you want using inst->addDir()
       etc, start the job through addInst().

       Parameter 'destDir' specifies where to install files.

       If parameter 'doAsk' is true, it means you are equipped to
       handle user-ask requests. It is important that you regularly
       check and respond to (or abort) requests from the ask queue
       through getNextError(), otherwise the job will hang while
       waiting for a response. If set to false, the job will overwrite
       and deleting files as necessary without asking.
     */
    InstallerPtr createInstaller(const std::string &destDir, RuleSet &rules,
                                 bool doAsk = false);
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
  };

  typedef boost::shared_ptr<JobManager> JobManagerPtr;
}
#endif
