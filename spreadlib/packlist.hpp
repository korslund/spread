#ifndef __SPREADLIB_PACKLIST_HPP_
#define __SPREADLIB_PACKLIST_HPP_

#include "packinfo.hpp"
#include <boost/shared_ptr.hpp>

/* This is the class responsible for loading and holding the
   packs.json data.
 */

namespace Spread
{
  struct PackList
  {
    PackList();

    // Return list of all packages
    const PackInfoList &getList() const;

    // Get one package, throw if it doesn't exist
    const PackInfo &get(const std::string &pack) const;

    /* Load packlist from a json file. The PackList is reusable, so
       loadJson() may be called multiple times. Each call clears out
       all existing data before loading the new information.
     */
    void loadJson(const std::string &file, const std::string &channel);

    /* Clear out all existing pack data, reverting the object back to
       the same state as right after it was constructed.
     */
    void clear();

  private:
    struct _Internal;
    boost::shared_ptr<_Internal> ptr;
  };
};

#endif
