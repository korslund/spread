#include "hashtask.hpp"

#include "job/thread.hpp"
#include "hash/hash_stream.hpp"
#include "tasks/multi.hpp"

#include <boost/filesystem.hpp>
#include <assert.h>
#include <stdexcept>

using namespace Spread;
using namespace Jobify;
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
  JobInfoPtr info;

  HashStreamPtr curStream;
  Hash curHash;
  std::string curFile;
};

HashTask::HashTask()
{
  ptr = new _HashTaskHidden;
}

HashTask::~HashTask()
{
  delete ptr;
}

struct CloseTask : Job
{
  HashTask *owner;

  CloseTask(HashTask *h)
    : owner(h) { assert(owner); }

  void doJob()
  {
    owner->getOutStream(Hash());
    setDone();
  }
};

JobInfoPtr HashTask::run(bool async)
{
  Tasks::MultiTask *j = new Tasks::MultiTask;
  Job *jjj = createJob();
  assert(jjj);
  j->add(jjj);
  /* Make sure we close up shop (call closeStream()) in the worker
     thread. That function call may perform some potentially
     time-consuming file copying, and we don't want it to block the
     calling thread needlessly.

     The MultiTask struct is a good way to ensure that this is done
     while the JobInfo status is still marked as 'busy'. This is
     essential, otherwise finish() might get called before the last
     closeStream() has finished.

     The second 'false' parameter to MultiTask::add() prevents
     CloseTask from messing up the job progress numbers.
   */
  j->add(new CloseTask(this), false);

  assert(!ptr->info);
  ptr->info = j->getInfo();
  assert(ptr->info);

  Thread::run(j,async);

  // Call finish ourselves if the job is already done
  if(!async)
    {
      assert(ptr->info->isFinished());
      finish();
    }

  return ptr->info;
}

void HashTask::finish()
{
  // TODO: Fix some more informative error reporting. Child classes
  // are bound to have more useful info about what went wrong.

  assert(ptr->info->isFinished());

  if(!ptr->info->isSuccess())
    fail(ptr->info->message);

  // Make sure the last stream was closed
  assert(!ptr->curStream);

  // Check that all outputs were satisfied
  if(outputs.size() != 0)
    fail("Task did not produce all requested output files.");
}

void HashTask::closeStream()
{
  if(!ptr->curStream) return;

  // Don't bother copying files or error checking the data if the job
  // itself has already failed
  if(ptr->info->isNonSuccess())
    return;

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
  HashStreamPtr res;

  // Undocumented function: Calling with a null hash will exit with no
  // pointer (as expected), but it will FIRST call closeStream().
  closeStream();
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
