#ifndef __SPREAD_HPP_
#define __SPREAD_HPP_

#include "job/jobinfo.hpp"
#include <iostream>

/* Top-level interface to the Spread system.
 */
namespace Spread
{
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

  private:
    struct _Internal;
    boost::shared_ptr<_Internal> ptr;
  };
}

#endif
