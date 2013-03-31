#ifndef __SPREADLIB_CHANLIST_HPP_
#define __SPREADLIB_CHANLIST_HPP_

#include "packlist.hpp"
#include "statuslist.hpp"
#include <boost/shared_ptr.hpp>
#include <rules/ruleset.hpp>
#include <job/jobinfo.hpp>

namespace Spread
{
  struct ChanList
  {
    ChanList(const std::string &basedir, RuleSet &rules);

    const PackList &getPackList(const std::string &channel);
    StatusList &getStatusList();

    /* Load existing disk data for the given channel, if any. Unless
       you are planning to overwrite the data on disk (see
       setChannelJob()), you usually don't need to call this as lazy
       loading is implicit in getPackList().
     */
    void load(const std::string &channel);

    /* Associate a loader job with a given channel. The corresponding
       channel data will not be loaded from disk until the job has
       finished successfully (job->isSuccess()).

       If the job fails, you must call setChannelJob() again with a
       new job and let that succeed, otherwise the channel will be
       blocked from ever loading.

       You may pre-load existing data on disk using load() BEFORE
       starting the job. This will load old data from disk (if any)
       into memory for use while you are waiting for the job to
       finish.
     */
    void setChannelJob(const std::string &channel, JobInfoPtr job);

  private:
    struct _Internal;
    boost::shared_ptr<_Internal> ptr;
  };
};

#endif
