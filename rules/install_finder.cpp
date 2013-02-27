#include "install_finder.hpp"
#include "arcrule.hpp"
#include <assert.h>
#include <boost/filesystem.hpp>
#include <set>

using namespace Spread;
namespace bf = boost::filesystem;

//#define PRINT_DEBUG
#ifdef PRINT_DEBUG
#include <iostream>
#define PRINT(a) std::cout << __LINE__ << ": " << a << "\n";
#else
#define PRINT(a)
#endif

InstallFinder::InstallFinder(const RuleFinder &_rules, Cache::CacheIndex &_cache)
  : rules(_rules), cache(_cache) {}

static std::string abs(const bf::path &file)
{ return bf::absolute(file).string(); }

bool InstallFinder::handleDeps(const DepList &deps, ActionMap &output, bool baseLevel)
{
  PRINT("ENTER handleDeps()");

  bool isOk = true;

  // On the top level we also handle the blind unpack actions.
  if(baseLevel)
    {
      PRINT("Handling " << blinds.size() << " blind unpack(s)");
      DepList subdeps;
      for(int i=0; i<blinds.size(); i++)
        {
          std::string dir = blinds[i].first;
          const Hash &hash = blinds[i].second;

          assert(dir != "");
          dir = abs(dir);

          // Add the archive hash as a dependency
          subdeps.push_back(DepPair("",hash));

          /* Create a dummy hash to represent the archive. This isn't
             looked up anywhere and is never used as a dependency, so
             any unique hash that avoids collisions will do.
          */
          Hash dummy("ARC_" + hash.toString().substr(0,4));

          // TODO: This leaks memory. Fix later.
          ArcRule *rule = new ArcRule(hash, "Blind Archive Unpack Rule");

          // Finally add the Action representing the unpack itself.
          output[dummy] = Action(rule, dir);
        }

      // Expand the dependencies
      if(!handleDeps(subdeps, output))
        isOk = false;
    }

  for(int i=0; i<deps.size(); i++)
    {
      PRINT("LOOP");

      std::string dest = deps[i].first;

      if(dest != "")
        dest = abs(dest);

      const Hash &hash = deps[i].second;

      PRINT("DEST=" << dest << " HASH=" << hash);

      // Check if this hash has already been resolved.
      ActionMap::iterator it = output.find(hash);
      if(it != output.end())
        {
          // Add ourself to it's destination list, if we have one
          it->second.addDest(dest);
          continue;
        }

      // CUT OUT FROM HERE

      if(!handleDeps(subdeps, output))
        isOk = false;

          /* TODO: Handle recursion. Recursion happens when a hash
             implicitly depends upon itself. We would also have to
             figure out what to do in the case of recursion. At the
             moment though, we just assume our ruleset is acyclic.
          */
          continue;
        }

      PRINT("NO MATCH FOUND");

      // There was no way to resolve this hash. An empty copy action
      // represents an unhandled dependency.
      output[hash] = Action("", dest);
      isOk = false;

      PRINT("END LOOP\n");
    }

  PRINT("Returning " << isOk);
  return isOk;
}
