#ifndef __HASH_COPYTASK_HPP_
#define __HASH_COPYTASK_HPP_

#include "hashtask.hpp"

/* This class copy one file to a given output set. The hash is guessed
   from the first entry in the copy list.
 */

namespace Spread
{
  struct CopyHash : HashTask
  {
    std::string source;

  private:
    Jobify::Job *createJob();
  };
};

#endif
