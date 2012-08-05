#ifndef __SPREAD_INSTALL_ACTIONINSTALLER_HPP
#define __SPREAD_INSTALL_ACTIONINSTALLER_HPP

#include "rules/actions.hpp"
#include "job/job.hpp"

/* Backend installer structure. Takes an input ActionMap, and turns it
   into results in the file system.

   The class is constructed to be used as a base class, with abstract
   virtual functions working as the connection to the outside world.
 */

namespace Spread
{
  struct ActionInstaller : Job
  {
    /* Report a broken URL.

       Returns the replacement URL, or an empty string if there is no
       replacement.
     */
    virtual std::string brokenURL(const Hash &hash, const std::string &url) = 0;

    /* Get the initial list of actions. This is only called once.
     */
    virtual void getActions(ActionMap &acts) = 0;

    /* Get a temporary location to write the given file hash.
     */
    virtual std::string getTmpFile(const Hash &h) = 0;

    /* Notify the system that a new file has been created.
     */
    virtual void addToCache(const Hash &h, const std::string &file) = 0;

  private:
    void doJob();
  };
}

#endif
