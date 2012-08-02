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

       Don't worry if the index contains more files than you need to
       extract. Files not requested with HashTask::addOutput() will
       still be ignored.
     */
    HashMap index;

    /* Generate an index from an archive file.

       An optional dir 'where' can be used to specify an output
       directory. If empty, no files are written.
     */
    static void makeIndex(const std::string &arcFile, HashMap &index,
                          const std::string &where = "");

  private:
    Jobs::Job *createJob();
    FileList list;
  };
};

#endif
