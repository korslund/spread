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

    TreeBase(TreeOwner &o);

    virtual void addOutput(const Hash &h, const std::string &where = "");
    virtual void addInput(const Hash &h);

    HashFinderPtr finder;

  protected:
    TreeOwner &owner;

    std::string setStatus(const std::string &msg);
    void fail(const std::string &msg);
    void log(const std::string &msg);

    /* Fetch all the files listed in 'outputs', and store them in the
       locations given (if any.) Every single file listed will be
       created and added to cache, or the function will throw an
       error.

       The 'results' map is filled with one file location for each
       hash in 'outputs'.

       For files with no location given, the returned/cached file may
       be a temporary file or a file that already existed before
       fetchFiles() was called.
     */
    void fetchFiles(const HashDir &outputs, HashMap &results);

    // Single-file convenience version of fetchFiles()
    std::string fetchFile(const Hash &hash, const std::string &target="");
  };

  typedef boost::shared_ptr<TreeBase> TreePtr;

  struct TreeOwner
  {
    typedef boost::shared_ptr<void> Lock;

    virtual void log(const std::string &msg) = 0;

    virtual TreePtr copyTarget(const std::string &from) = 0;
    virtual TreePtr downloadTarget(const std::string &url) = 0;
    virtual TreePtr unpackTarget(const Hash &dir) = 0;
    virtual TreePtr unpackBlindTarget(const std::string &where,
                                      const Hash &dir = Hash()) = 0;

    /* Load a directory object from the given file. It is inserted
       into the global dir cache if necessary. The optional 'check'
       hash can be used to verify that the dirhash matches
       expectation.
    */
    virtual void loadDir(const std::string &file, Hash::DirMap &output,
                         const Hash &check = Hash()) = 0;

    // Store the given directory object in the global dir cache.
    virtual void storeDir(const Hash::DirMap &dir, const Hash &check = Hash()) = 0;

    // Get a temporary file name
    virtual std::string getTmpName(const Hash &hash) = 0;

    // Notify our owner that the following files are done.
    virtual void notifyFiles(const Hash::DirMap &files) = 0;

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
