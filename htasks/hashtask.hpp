#ifndef __HASH_TASK_HPP_
#define __HASH_TASK_HPP_

#include "job/job.hpp"
#include "hash/hash.hpp"
#include <mangle/stream/stream.hpp>
#include <map>

/* HashTask is the base class for jobs that depend on or produce a
   verified set of input/output data. Exactly what data is needed or
   produced depends on the child class.

   The task is given a set of zero or more input files, identified by
   hashes, which it may use in any way it likes. The task should
   signal an error if the given input is not sufficient.

   The task is also optionally given a set requested of output files,
   identified by hashes. The task is required to produce all of them;
   if it can't, it is an error. It is not an error however, if the
   task is able to produce data that is not requested.

   An output hash may be requested more than once. It should then be
   duplicated to all the requested locations.
 */

namespace Spread
{
  struct HashTask
  {
    typedef std::multimap<Hash, std::string> HashDir;
    typedef std::pair<Hash, std::string> HDValue;

    HashTask();
    virtual ~HashTask();

    /* Set these up before running the job. Do not touch after calling
       run().
     */
    HashDir inputs;
    HashDir outputs;

    void addInput(Hash h, const std::string &file)
    { inputs.insert(HDValue(h,file)); }

    void addOutput(Hash h, const std::string &file)
    { outputs.insert(HDValue(h,file)); }

    /* Run the task. Returns the JobInfo for the job. If async=true,
       the job is started in a background thread.

       When running with async=true, you have to keep this HashTask
       alive for the duration of the job, and call finish() when it is
       done running.

       You are not required to handle error states in the JobInfo,
       finish will convert these to exceptions automatically.
    */
    Jobify::JobInfoPtr run(bool async=true);

    /* Call this when a threaded job has finished running. Will throw
       an exception if errors were detected.
     */
    void finish();

    /* Helper function for child classes and jobs. Produces an output
       stream for the requested hash, or an empty ptr if nobody
       requested the data.

       This guarantees that all output locations are created properly,
       and that the written data matches the requested hash.

       It's OK to call this from a threaded job, as you are guaranteed
       that nobody else will touch this object while the job is
       running.

       Only one stream can be used at any given time, so make sure you
       write all the data to the stream before opening a new one.
     */
    Mangle::Stream::StreamPtr getOutStream(const Hash &h);

  protected:
    // Create the job that performs this task.
    virtual Jobify::Job *createJob() = 0;

  private:
    void closeStream();
    struct _HashTaskHidden;
    _HashTaskHidden *ptr;
  };
};

#endif
