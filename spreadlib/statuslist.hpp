#ifndef __SPREADLIB_STATUSLIST_HPP_
#define __SPREADLIB_STATUSLIST_HPP_

#include "statusinfo.hpp"
#include <boost/shared_ptr.hpp>

/* This class keeps a list of install locations for packages. You may
   add, remove or update entries, and changes are automatically synced
   to a config file.

   The class can also help you determine whether or not your installed
   packages are up-to-date through notifyNew().
 */

namespace Spread
{
  struct StatusList
  {
    StatusList(const std::string &confFile);

    /* Return list of all installed packages, optionally screening for
       a given channel, package name, and/or install location.
    */
    void getList(PackStatusList &output,
                 const std::string &channel = "",
                 const std::string &package = "",
                 const std::string &where = "") const;

    /* Get the status of a single package. If no location is given,
       the first one found (if any) is returned. Returns NULL if no
       matching install locations are found.
     */
    const PackStatus *get(const std::string &channel,
                          const std::string &pack,
                          const std::string &where = "") const;

    /* Set or update an entry in the list. Existing entries are only
       overwritten if 'channel', 'package' and 'where' match.

       Changes are automatically synced to confFile. Setting an entry
       will always reset 'needsUpdate' to false.
     */
    void setEntry(const PackInfo &info, const std::string &where);

    // Remove an entry from the list
    void remove(const std::string &channel, const std::string &pack,
                const std::string &where = "");

    /* Notify us about newly loaded packages. If a newly loaded
       package name/channel matches one or more installed entries, the
       'needsUpdate' field is set accordingly:

       If the installed dirs/paths vectors match EXACTLY with the new
       data (length, content and order), needsUpdate is set to
       false. Otherwise, needsUpdate is set to true.

       Notifications for non-installed packages are ignored, and
       packages not mentioned in the notification are unaffected.
     */
    void notifyNew(const PackInfo &info);
    void notifyNew(const PackInfoList &list);

  private:
    struct _Internal;
    boost::shared_ptr<_Internal> ptr;
  };
};

#endif
