#ifndef __SPREAD_HPP_
#define __SPREAD_HPP_

#include "job/jobinfo.hpp"
#include <iostream>

/* This is a dummy implementation of the final, top-level interface.
 */

namespace Spread
{
  struct SpreadLib
  {
    SpreadLib(const std::string &outDir, const std::string &tmpDir)
    {}

    JobInfoPtr dummy()
    {
      JobInfoPtr ptr(new JobInfo);
      ptr->setDone();
      return ptr;
    }

    JobInfoPtr updateFromURL(const std::string &channel,
                             const std::string &url)
    {
      using namespace std;
      cout << "Initialized channel '" << channel << " from " << url << endl;
      return dummy();
    }

    JobInfoPtr install(const std::string &channel,
                       const std::string &package,
                       const std::string &where)
    {
      using namespace std;
      cout << "Installing " << channel << "::" << package << " to " << where << endl;
      return dummy();
    }

    /* Unpack the contents of a Spread link directly into the given
       location.
     */
    JobInfoPtr unpackURL(const std::string &url, const std::string &where)
    {
      return dummy();
    }

    /*
      Return package hash, as a string.

      The hash is as machine-readable summary hash of the package.
      Not only does every package have a unique hash, but everyr
      RELEASE of each package is also unique.

      You can store this when installing packages, and later compare
      against getPackageHash() to see if a package has been updated.

      The optional output string 'version' gives a human-readable
      version string, which may be updated for new versions. Don't use
      it for anything except displaying the package version to the
      user, as the string may be empty.
     */
    std::string getPackageHash(const std::string &channel, const std::string &package,
                               std::string *version = NULL) const
    {
      if(version) *version = "1.2.3";
      return "blah";
    }
  };
}

#endif
