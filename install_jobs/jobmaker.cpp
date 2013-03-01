#include "jobmaker.hpp"

#include <htasks/copyhash.hpp>
#include <htasks/downloadhash.hpp>
#include <htasks/unpackhash.hpp>

using namespace Spread;

HashTaskBase* JobMaker::copyJob(const std::string &from)
{
  return new CopyHash(from);
}

HashTaskBase* JobMaker::downloadJob(const std::string &url)
{
  /* TODO: Create a download queue system so the user can control how
     many simultaneous downloads they want to allow.
  */
  return new DownloadHash(url);
}

HashTaskBase* JobMaker::unpackJob(const Hash::DirMap &index)
{
  /* TODO: We might also cap the number of simultaneous unpacks, since
     more than one will likely just lead to disk thrashing and
     inefficient CPU usage.
   */
  return new UnpackHash(index);
}
