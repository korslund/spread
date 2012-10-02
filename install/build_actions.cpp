#include "build_actions.hpp"

#include "rules/install_finder.hpp"
#include <stdexcept>
#include <boost/filesystem.hpp>

using namespace Spread;

//#define DEBUG_PRINT
#ifdef DEBUG_PRINT
#include <iostream>
#define PRINT(a) std::cout << __LINE__ << ": " << a << "\n";

static void print(ActionMap &output)
{
  using namespace std;
  cout << "Got " << output.size() << " actions:\n";
  ActionMap::iterator it;
  for(it = output.begin(); it != output.end(); it++)
    {
      Action &a = it->second;

      cout << it->first << " : ";

      if(a.isNone())
        {
          cout << "UNHANDLED\n";
        }
      else if(a.isRule())
        {
          const Rule &r = *a.rule;

          cout << r.ruleString << endl;

          // Print rule dependencies
          for(int i=0; i<r.deps.size(); i++)
            cout << "  <= " << r.deps[i] << endl;
        }
      else if(a.isCopy())
        {
          cout << "COPY from " << a.source << endl;
        }
      else assert(0);

      // Print output files
      std::set<std::string>::iterator it2;
      for(it2 = a.destlist.begin(); it2 != a.destlist.end(); it2++)
        cout << "  => " << *it2 << endl;
    }
}
#else
#define PRINT(a)
#endif

void ActionBuilder::addHint(const Hash &hint)
{
  // Find the matching archive
  const ArcRuleData *data = rules.findArchive(hint);
  if(!data) return;

  // Load the directory object
  DirectoryCPtr dir = cache.loadDir(data->dirHash);
  if(!dir) return;

  // Prime the ArcRuleSet with the data
  arcs.addArchive(data->arcHash, dir, data->ruleString);
}

void ActionBuilder::addDep(const std::string &file, const Hash &hash)
{
  list.dir[file] = hash;
}

void ActionBuilder::addDir(const Directory *dir)
{
  Directory::DirMap::const_iterator it;
  for(it = dir->dir.begin(); it != dir->dir.end(); it++)
    addDep(it->first, it->second);
}

void ActionBuilder::addDir(const Hash &search, bool alsoAsHint)
{
  PRINT("addDir(hash=" << hash << ", alsoAsHint=" << alsoAsHint << ")");

  // Check if this is an archive hash, if so, pick out the dir hash.
  Hash dirHash = search;
  const ArcRuleData *data = rules.findArchive(search);
  if(data)
    dirHash = data->dirHash;

  DirectoryCPtr dir = cache.loadDir(dirHash);

  if(!dir)
    {
      PRINT("  No dir found, going blind.");

      // Add blind. Requires a matching archive rule.
      if(!data)
        throw std::runtime_error("Hash " + search.toString() + " matches neither a directory file nor any archive rule.");

      PRINT("  Adding archive " << data->arcHash);
      blind.push_back(data->arcHash);
      return;
    }

  PRINT("  Found directory with " << dir->dir.size() << " elements");

  addDir(dir);

  // Add the hint if the user requested it
  if(alsoAsHint && data)
    arcs.addArchive(data->arcHash, dir, data->ruleString);
}

void ActionBuilder::build(ActionMap &output)
{
  PRINT("build()");

  InstallFinder finder(arcs, cache.index);

  namespace bs = boost::filesystem;

  PRINT("DEPLIST");

  // Build dependency list
  {
    bs::path pdir = prefix;
    Directory::DirMap::const_iterator it;
    for(it = list.dir.begin(); it != list.dir.end(); it++)
      {
        // Add path prefix, if any
        bs::path file;
        if(prefix != "")
          file = pdir/it->first;
        else
          file = it->first;

        finder.addDep(file.string(), it->second);
      }
  }

  // Add "blind" archives
  for(int i=0; i<blind.size(); i++)
    finder.addBlind(prefix, blind[i]);

  PRINT("PERFORMING");

  bool res = finder.perform(output);

#ifdef DEBUG_PRINT
  print(output);
#endif

  if(!res)
    {
      // Build an error message
      std::string err = "Don't know how to satisfy the following dependencies:";

      // Loop through output and add the names of all the missing files
      ActionMap::const_iterator it;
      for(it = output.begin(); it != output.end(); it++)
        {
          const Action &a = it->second;
          if(a.isNone())
            {
              err += "\n  " + it->first.toString();
              std::set<std::string>::const_iterator it2;
              for(it2 = a.destlist.begin(); it2 != a.destlist.end(); it2++)
                err += "\n    " + *it2;
            }
        }

      throw std::runtime_error(err);
    }
}
