#ifndef __SPREAD_INSTALL_TREEBASE_HPP_
#define __SPREAD_INSTALL_TREEBASE_HPP_

#include <parent_job/execjob.hpp>
#include "ihashfinder.hpp"
#include <map>

namespace Spread
{
  struct TreeOwner;
  struct TreeBase : ExecJob
  {
    typedef std::multimap<Hash, std::string> HashDir;
    typedef std::pair<Hash, std::string> HDValue;
    typedef std::map<Hash, std::string> HashMap;

    TreeBase(TreeOwner &o, IHashFinder &f);
    TreeBase(TreeOwner &o);

    virtual void addOutput(const Hash &h, const std::string &where = "");

  protected:
    TreeOwner &owner;
    IHashFinder &finder;

    /* Fetch all the files listed in 'outputs', and store them in the
       locations given (if any.) Every single file listed will be
       created and added to cache, or the function will throw an
       error.

       If the 'results' map is given, it will be filled with one file
       location for each hash in 'outputs'.

       For files with no location given, the returned/cached file may
       be a temporary file or a file that already existed before
       fetchFiles() was called.
     */
    void fetchFiles(const HashDir &outputs, HashMap *results = NULL);
  };

  typedef boost::shared_ptr<TreeBase> TreePtr;

  struct TreeOwner
  {
    typedef boost::shared_ptr<void> Lock;
    IHashFinder &finder;

    TreeOwner(IHashFinder &f) : finder(f) {}

    virtual TreePtr copyTarget(const std::string &from) = 0;
    virtual TreePtr downloadTarget(const std::string &url) = 0;
    virtual TreePtr unpackTarget(const Hash &arc, const Hash &dir) = 0;

    // Notify our owner that the following files are done.
    virtual void notifyFiles(const Hash::DirMap &files) = 0;

    // Get a temporary file name
    virtual std::string getTmpName(const Hash &hash) = 0;

    /* Check for existing jobs associated with the target hash.
       Returns an empty JobInfoPtr if no job was found.
     */
    virtual JobInfoPtr getRunningTarget(const Hash &hash) = 0;

    /* Insert a new job associated with a target hash. May not be
       called if there is already an existing job associated with this
       hash.
     */
    virtual void setRunningTarget(const Hash &hash, JobInfoPtr ptr) = 0;

    /* Create a lock pointer. This guarantees that no targets are
       added or removed from the target list while the lock is in
       effect.
     */
    virtual Lock lock() = 0;
  };
}

#endif
