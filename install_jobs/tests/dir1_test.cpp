/* Test a dummy dir job (installs a list of files into a preset
   location.)

   Uses dummies to simulate external systems, so no actual files are
   read, downloaded or created.
 */

#include <iostream>
using namespace std;

using namespace Spread;

struct InstallTest : ExecJob
{
  typedef std::multimap<Hash, std::string> HashDir;
  typedef std::pair<Hash, std::string> HDValue;
  typedef std::map<Hash, std::string> HashMap;

  void addOutput(const Hash &h, const std::string &file="")
  { outputs.insert(HDValue(h,file)); }

  void fetchOutputsDRAFT1()
  {
    // Not perfect but it does the job. Fix later.
    RuleFinder *rf = rulePtr;
    if(!rf) rf = &rules;

    JobMap jobs;
    InfoMap waitList;

    for(hash, name; outputs)
      {
        // This part is basically the old fetchFile()

        int stat = getStatus(hash, name);
        HashTaskBase *job = NULL;
        if(stat == InPlace)
          continue;

        // Abort if we are already waiting for this target
        if(waitList.has(hash))
          continue;

        // Get existing job if any
        if(jobs.has(hash))
          job = jobs[hash];

        else if(stat == File)
          {
            if(name == "")
              // Accept existing location
              continue;

            // Otherwise, set up a copy job. Output is added below.
            job = Copy(cacheName);
          }

        if(!job)
          {
            // No file was found. Check if there is a system-wide job
            // already running.
            inf = getRunningJob(hash);
            if(inf)
              {
                waitList.add(hash, inf);
                continue;
              }

            // No job, no file. Create based on rules.
            assert(stat == Archive || stat == Download);

            job = createWhatever(stat, etc);
            job->addInput(arcHash, whatever);
            insertRunningJob(job);
          }

        assert(job);

        /* We ASSUME that our subjob is capable of obtaining the files
           it needs itself, through the same process as we are using
           here.
        */

        if(jobs.has(hash)) assert(jobs[hash] == job);
        else jobs[hash] = job;

        job->addOutput(hash);
      }

    if(jobs.size())
      {
        // Run all jobs until success
        AndJob *aj = new AndJob;
        for(j; jobs)
          aj->add(j);
        execJob(aj); // Handles errors for us
      }

    if(waitList.size())
      {
        // Wait until all jobs have finished. Ignore errors.
        for(inf; waitList)
          inf.wait(getInfo());
      }

    /* OK, so at this point we get the same restart problem as we had
       earlier. All jobs may have finished, we may or may not have
       written all the locations we want, we assume (but haven't
       checked yet) that all files exist in cache. For the waitList
       jobs it is pretty certain that we have NOT created our wanted
       target locations, if any.

       One solution is this:

       - above, do NOT create copy jobs at all. Just ignore EVERYTHING
         in cache. JUST deal with the cases of
     */

    outputs.clear();
  }

  // None of these parameters have any business being class variables!
  void fetchOutputsDRAFT2(const HashDir &outputs, HashMap &results, RuleFinder *rulePtr)
  {
    RuleFinder *rf = rulePtr;
    if(!rf) rf = &rules;

    InfoMap waitList;
    AndJob *aj = NULL;

    MovableLock lock = getListLock();
    for(hash, name; outputs)
      {
        // Abort if we have already looked up this hash
        if(waitList.has(hash))
          continue;

        // No file was found. Check if there is a system-wide job
        // already running.
        {
          JobInfoPtr inf = getRunningJob(hash);
          if(inf)
            {
              waitList.add(hash, inf);
              continue;
            }
        }

        int stat = getStatus(hash, name);
        if(stat == File || stat == InPlace)
          continue;

        // Probably use a shared_ptr here. And actually, like before
        // we make a Target first, then set it up to do what we want.
        HashTaskBase *job = NULL;
        if(stat == Download)
          job = createWhatever();
        else if(stat == Archive)
          {
            // TODO: Get dirhash etc first

            // Check if an archive job already exists. TODO: Not
            // global, just local.
            inf = global.getArchive(arcHash, hash, target);
            if(inf)
              {
                // So this is more like: do our local list have a
                // Target stored for this arcHash? If so, then just do
                // this:
                arcList[arcHash]->addOutput(hash, target);
                continue;
              }

            // No existing archive job. Create a new one.
            job = createWhatever();
            global.insertArchive(job, arcHash);

            // TODO: Ok, so this job needs to recursively look for the
            // archive itself. Which means it is NOT a normal
            // UnpackHash job. It is still one of our special Target
            // jobs (which we might be too.)
            job->addInput(arcHash, whatever);

            // This should now work
            inf = global.getArchive(arcHash, hash, target);
            assert(inf);
          }
        else assert(0);

        assert(job);

        // Let the outside world know about our target
        setRunningJob(hash, job->getInfo());

        // TODO: This crashes with the above 'global' stuff, but we
        // won't make that global anyway.
        job->addOutput(hash);

        if(!aj) aj = new AndJob;
        aj->add(job);
        waitList.add(job->getInfo(););
      }
    lock.reset();

    execJob(aj);

    if(waitList.size())
      {
        // Wait until all jobs have finished. Ignore errors.
        for(inf; waitList)
          inf.wait(getInfo());
      }

    /* Finally check all the cache values, and set up 'results' based
       on it. Fail if anything is missing.

       If we get File instead of InPlace, then create a copy operation
       and execJob it directly (no point in multitasking file copies
       really.) But use JobMaker still so we can abstract it.
     */
  }

  void doJob()
  {
    /* Request initial objects not part of the install itself. We
       don't specify a target location, so tmp or existing files are
       ok.
     */
    addOutput(dirHash);
    addOutput(arcFile);
    fetchOutputs();

    // Load a directory file, possibly caching if necessary. Throws on
    // error.
    DirectoryCPtr dir = system.loadDir(dirHash, results[dirHash]);

    LocalRules myrules(rules);
    myrules.doBlahBlah();
    rulePtr = &myrules;

    // Add final targets (m syntax :)
    for(name, hash; dir->dir)
      addOutput(hash, name);

    fetchOutputs();
    setDone();
  }
};
