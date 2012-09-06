#ifndef __SPREAD_HPP_
#define __SPREAD_HPP_

#include "job/jobinfo.hpp"
#include <boost/function.hpp>

/* Top-level interface to the Spread system.
 */
namespace Spread
{
  struct Hash;
  struct SpreadLib
  {
    SpreadLib(const std::string &outDir, const std::string &tmpDir);

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
       are created as necessary.

       If a version string is given, it is set with the package's
       version description, if any.

       If async=true, returns the job controller for the given job.

       Throws an exception if the package or channel does not
       exist. All other errors are returned as error statuses in the
       JobInfo.
     */
    JobInfoPtr install(const std::string &channel,
                       const std::string &package,
                       const std::string &where,
                       std::string *version = NULL,
                       bool async=true);

    /* Unpack the contents of a Spread SR0 url directly into the given
       location.
     */
    JobInfoPtr unpackURL(const std::string &url, const std::string &where,
                         bool async=true);

    /* Add a file to the local file cache. Any future requests for
       this data (as identified by the file's hashed value) will be
       copied from this location, instead of being downloaded or
       unpacked from other sources.
     */
    std::string cacheFile(const std::string &file);

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
