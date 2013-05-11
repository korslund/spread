#include "downloadhash.hpp"
#include "tasks/download.hpp"

using namespace Spread;

Job* DownloadHash::createJob()
{
  desc = "downloading " + url;

  // Guess the hash from targets
  assert(outputs.size() > 0);
  assert(url != "");
  const Hash &hash = outputs.begin()->first;
  return new DownloadTask(url, getOutStream(hash));
}
