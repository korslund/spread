#include "hashtask.hpp"

#include "hash/hash_stream.hpp"
#include <boost/filesystem.hpp>
#include <assert.h>
#include <stdexcept>

using namespace Spread;
using namespace Mangle::Stream;
namespace bs = boost::filesystem;

typedef HashTask::HashDir::iterator HDI;
typedef std::pair<HDI,HDI> HDP;

// Makes sure all parent directories of a file exists
static void parent(const bs::path &file)
{
  bs::create_directories(file.parent_path());
}

static void fail(const std::string &msg)
{
  throw std::runtime_error(msg);
}

struct HashTask::_HashTaskHidden
{
  HashStreamPtr curStream;
  Hash curHash;
  std::string curFile;
};

HashTask::HashTask()
{
  ptr.reset(new _HashTaskHidden);
}

void HashTask::doJob()
{
  Job *job = createJob();
  assert(job);
  boost::shared_ptr<Job> deleter(job);

  if(runClient(*job)) return;
  closeStream();

  // Check that all outputs were satisfied
  if(outputs.size() != 0)
    setError("Task did not produce all requested output files.");
  else
    setDone();
}

void HashTask::closeStream()
{
  if(!ptr->curStream) return;

  assert(info->isBusy());

  // Hash the written data
  Hash res = ptr->curStream->finish();

  // Make sure the stream is flushed, since we might copy the file below
  ptr->curStream->flush();

  // Clear our stream pointer
  ptr->curStream.reset();

  if(res != ptr->curHash)
    fail("Hash mismatch in " + ptr->curFile +
         "\n  Expected: " + ptr->curHash.toString() +
         "\n  Recieved: " + res.toString());

  // Loop through the output list and find the rest of the outputs for
  // this hash, if any.
  HDP range = outputs.equal_range(res);
  for(HDI it = range.first; it != range.second; ++it)
    {
      std::string file = it->second;
      assert(it->first == res);
      assert(file != ptr->curFile);

      // Copy the file into place from the original
      if(bs::exists(file)) bs::remove(file);
      parent(file);
      bs::copy_file(ptr->curFile, file);
    }

  // Remove all the entries from the output list
  outputs.erase(range.first, range.second);
}

StreamPtr HashTask::getOutStream(const Hash &h)
{
  closeStream();

  HashStreamPtr res;
  if(h.isNull()) return res;

  // Is this a requested hash?
  HDI it = outputs.find(h);
  if(it == outputs.end())
    /* Nope. Either nobody wanted this data, or it has already been
       extracted (and removed from the list.) In either case, don't
       bother extracting it.
     */
    return res;

  /* Fetch the output file. This will only fetch ONE of potentially
     any number of output files for this hash. But that's good enough,
     we will copy into the others if necessary.
  */
  std::string file = it->second;

  // Open the file for output, wrapped in a HashStream
  parent(file);
  res.reset(new HashStream(file, true));

  // Remove the entry from the outputs table, so we won't find it
  // again.
  outputs.erase(it);

  // Store the results so we can check them later
  ptr->curStream = res;
  ptr->curHash = h;
  ptr->curFile = file;
  return res;
}
