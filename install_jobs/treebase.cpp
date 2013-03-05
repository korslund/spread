#include "treebase.hpp"
#include <parent_job/andjob.hpp>
#include <stdexcept>

using namespace Spread;

TreeBase::TreeBase(TreeOwner &o, IHashFinder &f)
  : owner(o), finder(f)
{}

TreeBase::TreeBase(TreeOwner &o)
  : owner(o), finder(o.finder)
{}

std::string TreeBase::setStatus(const std::string &msg)
{
  log("STATUS: " + msg);
  return setBusy(msg);
}
void TreeBase::fail(const std::string &msg)
{
  log("ERROR: " + msg);
  throw std::runtime_error(msg);
}
void TreeBase::log(const std::string &msg)
{ owner.log(msg); }


void TreeBase::addOutput(const Hash &h, const std::string &where) { assert(0); }
void TreeBase::addInput(const Hash &h) { assert(0); }

void TreeBase::fetchFiles(const HashDir &outputs, HashMap &results)
{
  std::string oldMsg = setStatus("Setting up child jobs");

  std::map<Hash,JobInfoPtr> waitList;
  AndJob *aj = NULL;

  {
    std::map<Hash,TreePtr> unpackers;
    HashDir::const_iterator it;

    TreeOwner::Lock lock = owner.lock();
    for(it = outputs.begin(); it != outputs.end(); it++)
      {
        const Hash &hash = it->first;
        const std::string &outfile = it->second;

        // Abort if we have already looked up this hash
        if(waitList.find(hash) != waitList.end())
          continue;

        // Check if there is a system-wide job already running for
        // this target.
        {
          JobInfoPtr inf = owner.getRunningTarget(hash);
          if(inf)
            {
              waitList[hash] = inf;
              continue;
            }
        }

        HashSource src;
        if(!finder.findHash(hash, src, outfile))
          fail("No source for target " + hash.toString() + " " + outfile);
        assert(src.hash == hash);

        // If the file already exists, deal with it later.
        if(src.type == TST_File || src.type == TST_InPlace)
          continue;

        // Probably use a shared_ptr here. And actually, like
        // before we make a Target first, then set it up to do
        // what we want.
        TreePtr job;
        if(src.type == TST_Download)
          job = owner.downloadTarget(src.value);
        else if(src.type == TST_Archive)
          {
            assert(!src.dirHash.isNull());
            assert(src.value == "");
            assert(src.deps.size() == 1);
            Hash arcHash = src.deps[0];

            // Check if we have already created an archive
            // unpacker for this archive
            if(unpackers.find(arcHash) != unpackers.end())
              {
                // Add this target to the output list of the
                // already existing unpacker.
                unpackers[arcHash]->addOutput(hash, outfile);
                continue;
              }

            job = owner.unpackTarget(src.dirHash);
            unpackers[arcHash] = job;
          }
        else assert(0);

        assert(job);

        // Add this target to the jobs output list
        job->addOutput(hash, outfile);

        // Add input dependencies
        for(int i=0; i<src.deps.size(); i++)
          job->addInput(src.deps[i]);

        // Let the outside world know about our target
        owner.setRunningTarget(hash, job->getInfo());

        // And add the job to our local execution list
        if(!aj) aj = new AndJob;
        aj->add(job);
      }
    // 'lock' goes out of scope here
  }

  // This will run all our jobs, and automatically fail if any of them fail.
  if(aj)
    execJob(aj);

  // Wait for external jobs to finish
  std::map<Hash,JobInfoPtr>::const_iterator wit;
  for(wit = waitList.begin(); wit != waitList.end(); wit++)
    {
      setStatus("Waiting for target " + wit->first.toString());
      wit->second->wait(getInfo());
    }

  // Finally check all the cache values, and set up 'results'
  // based on what we find. Fail if anything is missing.
  HashDir::const_iterator it;
  results.clear();
  for(it = outputs.begin(); it != outputs.end(); it++)
    {
      const Hash &hash = it->first;
      const std::string &outfile = it->second;

      // Look up and check if the requested file has been created
      // somewhere.
      HashSource src;
      if(!finder.findHash(hash, src, outfile) ||
         (src.type != TST_File && src.type != TST_InPlace))
        fail("Failed to create target " + hash.toString() + " " + outfile);
      assert(src.hash == hash);
      assert(src.value != "");

      // Copy the file into place if necessary
      if(src.type == TST_File && outfile != "")
        {
          TreePtr job = owner.copyTarget(src.value);
          job->addOutput(hash, outfile);
          execJob(job);
          src.value = outfile;
        }
      assert(src.value != "");

      // Store the result
      results[hash] = src.value;
    }

  setStatus(oldMsg);
}
