#ifndef __HASH_COPYTASK_HPP_
#define __HASH_COPYTASK_HPP_

#include "hashtask.hpp"

/* This class copies one file to a given output set. The hash is
   guessed from the first entry in the output list added through
   HashTask::addOutput(). All outputs must be of the same hash.
 */

namespace Spread
{
  struct CopyHash : HashTask
  {
    CopyHash(const std::string &_source)
      : source(_source) {}

  private:
    std::string source;
    Job *createJob();
  };
};

#endif
