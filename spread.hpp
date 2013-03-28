#ifndef __SPREAD_HPP_
#define __SPREAD_HPP_

#include "job/jobinfo.hpp"
#include "install_system/jobmanager.hpp"
#include <boost/function.hpp>
#include <vector>

/* Top-level interface to the Spread system.
 */
namespace Spread
{
  struct Hash;
  struct SpreadLib
  {
    typedef std::vector<std::string> StrVec;

    struct PackInfo
    {
      std::string channel, package, version;
      StrVec dirs, paths;
      uint64_t installSize;
    };

    struct PackStatus
    {
      PackInfo *pack;
      std::string path, curVer;
      StrVec dirs, paths;
      bool isUpdated;
    };

    typedef std::vector<PackInfo> InfoList;
    typedef std::list<const PackStatus*> StatusList;

    SpreadLib(const std::string &outDir, const std::string &tmpDir);

    JobManagerPtr getJobManager() const;

    /* Update the current rule set and package list from the given
       source.
     */
    JobInfoPtr updateFromURL(const std::string &channel,
                             const std::string &url,
                             bool async=true);

    JobInfoPtr updateFromFile(const std::string &channel,
                              const std::string &path,
                              bool async=true);

    /* Returns whether any new data was downloaded in the last call to
       updateFromURL()/File() for this channel.

       Only call this after the appropriate job has finished.
    */
    bool wasUpdated(const std::string &channel) const;

    /* Install the given package from 'channel' into the destination
       directory 'where'. All necessary sub- and parent-directories
       are created as necessary. Do not call while an updateFrom

       If a version string is given, it is set with the package's
       version description, if any.

       If async=true, returns the job controller for the given job.

       If doUpgrade=true, then we will upgrade the existing
       installation in place, if any. This means, among other things,
       that files which are not marked as changed in the upgrade will
       not be touched or reinstalled, even if changed/missing on
       disk. If there are no files to be upgraded, the function will
       return an empty JobInfoPtr. If doUpgrade=false, any existing
       installation in the same location is ignored. (The individual
       files on disk may still be reused by the installer subsystem
       though.)

       If enableAsk=true, file conflicts (eg. overwriting/deleting
       user-modified files) will be posted to a global message queue,
       which you can fetch through getJobManager()->getNextError().
       Note that you then MUST poll and answer all requests in the
       queue for as long as the job is running, or else the job might
       be left hanging while waiting for an answer.

       On success, the job system will remember the install location
       so that future calls to installPack() will update rather than
       reinstall, and getPackStatus()/getStatusList() knows about the
       installation.

       Throws an exception if the package or channel does not
       exist. All other errors are returned as error statuses in the
       JobInfo. May return an empty shared_ptr if there was no work to
       be done (installation is already up-to-date.)
     */
    JobInfoPtr installPack(const std::string &channel,
                           const std::string &package,
                           const std::string &where,
                           std::string *version = NULL,
                           bool async=true,
                           bool doUpgrade=true,
                           bool enableAsk=false);

    /* Uninstall a package previously installed with installPack().
       Files are deleted and the entry is removed from the database.

       The 'where' location is optional - if not set, the first
       channel/package match is selected.

       If no matching installation is found, an empty JobInfoPtr is
       returned.
     */
    JobInfoPtr uninstallPack(const std::string &channel,
                             const std::string &package,
                             const std::string &where = "",
                             bool async=true);


    /* Get general information about a package.
     */
    const PackInfo &getPackInfo(const std::string &channel,
                                const std::string &package) const;

    /* Get a list of all packages in a channel
     */
    const InfoList &getInfoList(const std::string &channel) const;

    /* Get installation-specific information about a package,
       including whether the given installation is out of date.

       An optional installation location can be given to specify the
       installation at path 'where'. If no path is given, the first
       matching installation in the list is returned.

       Returns NULL if no installation was found.
     */
    const PackStatus *getPackStatus(const std::string &channel,
                                    const std::string &package,
                                    const std::string &where = "") const;

    /* Get a list of all currently installed packages, optionally
       restricted to a given channel, package, and/or location.
     */
    const StatusList &getStatusList(StatusList &output,
                                    const std::string &channel = "",
                                    const std::string &package = "",
                                    const std::string &where = "") const;


    /* Unpack the contents of a Spread SR0 url directly into the given
       location.
     */
    JobInfoPtr unpackURL(const std::string &url, const std::string &where,
                         bool async=true);

    // Verify all entries in the file cache database
    void verifyCache();

    /* Add a file to the local file cache. Any future requests for
       this data (as identified by the file's hashed value) will be
       copied from this location, instead of being downloaded or
       unpacked from other sources.

       Returns a string version of the file hash.
     */
    std::string cacheFile(const std::string &file);

    /* Convenience function. Copy a file from one location to another,
       and add both the source and the destination to the cache
       index. This is more efficient than copying the file and calling
       cacheFile() manually, since the data is only read and cached
       once.

       Returns a string version of the file hash.
     */
    std::string cacheCopy(const std::string &from, const std::string &to);

    /* Copy a list of files to new destinations, adding all sources
       and destinations to the cache. When copying many files, this is
       much faster than running cacheCopy on each file individually,
       because it uses the faster CacheIndex::addMany() interface
       internally.

       The inputs and outputs vectors must be exactly the same
       length. Each file is copied from the path in inputs[] to the
       corresponding path in outputs[].

       An optional JobInfoPtr can be provided if the function is being
       run from a thread; cacheCopy() will call info->setProgress()
       with size information.
     */
    void cacheCopy(const std::vector<std::string> &inputs,
                   const std::vector<std::string> &outputs,
                   JobInfoPtr info = JobInfoPtr());

    /* Download a file from 'url' to the given location. This is a
       convenience function that is entirely independent of the rest
       of the Spread system, except for using Spread code to perform
       the download.

       Parent directories of 'dest' are created as necessary.
     */
    static JobInfoPtr download(const std::string &url,
                               const std::string &dest,
                               bool async=true);

    /* Set callback to handle broken URLs. This is invoked whenever
       the system encounters a non-working URL, or an URL for which
       the contents has changed. Setting the callback is optional.
     */
    typedef boost::function< void(const Hash &hash, const std::string &url) > CBFunc;
    void setURLCallback(CBFunc cb);

  private:
    struct _Internal;
    boost::shared_ptr<_Internal> ptr;
  };
}

#endif
