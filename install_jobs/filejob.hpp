#ifndef __SPREAD_FILEJOB_HPP_
#define __SPREAD_FILEJOB_HPP_

#include "target.hpp"
#include <install_system/ihashfinder.hpp>
#include <boost/thread/mutex.hpp>

namespace Spread
{
  typedef boost::shared_ptr<boost::lock_guard<boost::mutex> > MovableLock;

  /* System callback used by FileJob. All implementations must of
     course be fully thread safe.
   */
  struct FileJobOwner
  {
    // Notify our owner that the following files are done.
    virtual void notifyFiles(const Hash::DirMap &files) = 0;

    // Get a temporary file name
    virtual std::string getTmpName(const Hash &hash) = 0;

    /* Check for existing jobs associated with the target hash.

       - if one exists, the function returns true, overwrites the
         value in 'job'

       - if no target exists, return false. The given job is inserted
         into the list, and will be returned by future calls to
         getTarget().

       This function REQUIRES that the object is currently locked.
     */
    virtual bool getTarget(const Hash &hash, TargetPtr &job) = 0;

    /* Create a copyable lock object. This guarantees that no targets
       are added or removed from the target list while the lock is in
       effect. Unlock the list by letting the MovableLock go out of
       scope, or call ::reset() on it. (May return an empty pointer.)
     */
    virtual MovableLock lock() = 0;
  };

  struct FileJob : ExecJob, TargetOwner
  {
    FileJob(IHashFinder &fnd, FileJobOwner &own)
      : finder(fnd), owner(own) {}

    std::string outFile;

    std::string fetchFile(const Hash &hash, JobPtr &job,
                          const std::string &target="");

    void brokenURL(const Hash &hash, const std::string &url)
    { finder.brokenURL(hash, url); }

    void notifyFiles(const Hash::DirMap &files)
    {
      finder.addToCache(files);
      owner.notifyFiles(files);
    }

    void doJob();

  private:
    IHashFinder &finder;
    FileJobOwner &owner;
  };
}

#endif