#include "install_finder.hpp"
#include <boost/filesystem.hpp>
#include <set>

using namespace Spread;

InstallFinder::InstallFinder(const RuleFinder &_rules, Cache::CacheIndex &_cache)
  : rules(_rules), cache(_cache) {}

void InstallFinder::handleDeps(const DepList &deps, ActionMap &output)
{
  for(int i=0; i<deps.size(); i++)
    {
      const std::string &dest = deps[i].first;
      const Hash &hash = deps[i].second;

      // Check if this hash has already been resolved.
      ActionMap::iterator it = output.find(hash);
      if(it != output.end())
        {
          // Add ourself to it's destination list, if we have one
          it->second.addDest(dest);

          continue;
        }

      // Hash not previously resolved. Check if the cache knows
      // anything about the hash.
      int stat = Cache::CI_ElseWhere;
      if(dest != "")
        stat = cache.getStatus(dest, hash);

      if(stat == Cache::CI_Match)
        {
          /* Add copy action with empty destination. This allows other
             tasks to look up the same action later and add multiple
             destinations.

             If no other actions need this file, then the action is
             ignored, which is OK since the file is already installed.
          */
          output[hash] = Action(dest);
          continue;
        }

      if(stat == Cache::CI_ElseWhere)
        {
          // File either exists elsewhere, or dest == "". Get cache
          // location, if any.
          const std::string &existing = cache.findHash(hash);

          if(existing != "")
            {
              // Yup, it exists. Add a copy operation.
              output[hash] = Action(existing, dest);
              continue;
            }
        }

      /* If we are here, no existing file met our dependency. Look
         up the ruleset to see how else we can fetch the file.
      */
      const Rule *r = rules.findRule(hash);

      if(r)
        {
          bool found = false;

          // Set up the output action
          Action a(r);

          // Loop through and add the action for all the hashes it
          // produces, not just the one we are looking for.
          for(int k=0; k<r->outputs.size(); k++)
            {
              const Hash &h = r->outputs[k];
              output[h] = a;
              if(h == hash)
                {
                  // Add our output destination to the right output
                  output[h].addDest(dest);
                  found = true;
                }
            }
          assert(found);

          // Then loop through this rule's dependencies, and expand
          // those as well
          DepList subdeps;
          for(int k=0; k<r->deps.size(); k++)
            {
              const Hash &h = r->deps[k];
              subdeps.push_back(DepPair("",h));
              handleDeps(subdeps, output);
            }

          /* TODO: Handle recursion. Recursion happens when a hash
             implicitly depends upon itself. We would also have to
             figure out what to do in the case of recursion. At the
             moment though, we just assume our ruleset is acyclic.
          */
          continue;
        }

      // There was no way to resolve this hash. An empty copy action
      // represents an unhandled dependency.
      output[hash] = Action("", dest);
    }
}
