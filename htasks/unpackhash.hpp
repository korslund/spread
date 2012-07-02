#ifndef __HASH_UNPACKTASK_HPP_
#define __HASH_UNPACKTASK_HPP_

#include "hashtask.hpp"
#include <set>

/* This class unpacks one archive file, and writes outputs.

   - outputs is set as normal through HashTask::addOutput()
   - inputs must list exactly one archive file
   - index must be pre-generated with makeIndex()
 */

namespace Spread
{
  struct UnpackHash : HashTask
  {
    typedef std::map<std::string, Hash> HashMap;
    typedef std::set<std::string> FileList;

    /* Index used to look up the archive file. The string format is
       archive-type specific, so you should only supply an index
       created with makeIndex().
     */
    HashMap index;

    /* Generate an index from an archive file.
     */
    static void makeIndex(const std::string &arcFile,
                          HashMap &index);

  private:
    Jobify::Job *createJob();
    FileList list;
  };
};

#endif
