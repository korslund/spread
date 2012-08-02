#include "downloadhash.hpp"
#include "tasks/download.hpp"

using namespace Spread;

Jobs::Job* DownloadHash::createJob()
{
  // Guess the hash from targets
  assert(outputs.size() > 0);
  const Hash &hash = outputs.begin()->first;
  return new Tasks::DownloadTask(url, getOutStream(hash));
}
