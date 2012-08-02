#ifndef __HASH_DOWNLOADTASK_HPP_
#define __HASH_DOWNLOADTASK_HPP_

#include "hashtask.hpp"

/* This class downloads one file to a given output set. The hash is
   not set directly, instead it is guessed from the first entry in the
   download list.
 */

namespace Spread
{
  struct DownloadHash : HashTask
  {
    DownloadHash(const std::string &_url)
      : url(_url) {}

  private:
    std::string url;
    Jobs::Job *createJob();
  };
};

#endif
