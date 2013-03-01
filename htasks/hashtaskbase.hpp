#ifndef __HASH_TASK_BASE_HPP_
#define __HASH_TASK_BASE_HPP_

#include "job/job.hpp"
#include "hash/hash.hpp"
#include <map>

namespace Spread
{
  struct HashTaskBase : Job
  {
    typedef std::multimap<Hash, std::string> HashDir;
    typedef std::pair<Hash, std::string> HDValue;

    /* Set these up before running the job. Do not touch after calling
       run().
     */
    HashDir inputs;
    HashDir outputs;

    void addInput(Hash h, const std::string &file)
    { inputs.insert(HDValue(h,file)); }

    void addOutput(Hash h, const std::string &file)
    { outputs.insert(HDValue(h,file)); }
  };
};

#endif
