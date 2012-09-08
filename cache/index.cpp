#include "index.hpp"
#include <map>
#include <time.h>
#include <boost/filesystem.hpp>
#include "hash/hash_stream.hpp"
#include <stdint.h>
#include <boost/thread/recursive_mutex.hpp>
#include "misc/jconfig.hpp"

//#define PRINT_DEBUG

#ifdef PRINT_DEBUG
#include <iostream>
#define PRINT(a) std::cout << a << "\n";
#else
#define PRINT(a)
#endif

using namespace Cache;
using namespace Spread;
namespace bf = boost::filesystem;

static std::string abs(const bf::path &file)
{ return bf::absolute(file).string(); }

struct Entry
{
  Hash hash;
  std::string file;
  time_t writeTime;
};

typedef std::multimap<Hash, Entry*> HashToEntry;
typedef std::map<std::string, Entry*> PathToEntry;

typedef HashToEntry::iterator HTE_it;
typedef PathToEntry::iterator PTE_it;

typedef std::pair<Hash, Entry*> HTE_val;
typedef std::pair<HTE_it,HTE_it> HTE_pair;

typedef boost::lock_guard<boost::recursive_mutex> LOCK;

struct CacheIndex::_CacheIndex_Hidden
{
  PathToEntry paths;
  HashToEntry hashes;

  Misc::JConfig conf;
  boost::recursive_mutex mutex;

  void addConf(const std::string &file, const Hash &hash, time_t wtime)
  {
    char buf[16];
    snprintf(buf, 16, "%ld", wtime);
    conf.set(file, hash.toString() + " " + std::string(buf));
  }

  void removeConf(const std::string &file)
  { conf.remove(file); }

  void loadConf(const std::string &file)
  {
    conf.load(file);

    // Insert into the list
    std::vector<std::string> names;
    names = conf.getNames();

    for(int i=0; i<names.size(); i++)
      {
        const std::string &file = names[i];

        std::string val = conf.get(file);
        int split = val.find(' ');
        Hash hash(val.substr(0,split));
        time_t wtime = atoll(val.substr(split+1).c_str());
        add(file, hash, wtime);
      }
  }

  Entry *find(const std::string &file)
  {
    PTE_it it = paths.find(file);
    if(it == paths.end()) return NULL;
    return it->second;
  }

  Entry *find(const Hash &h)
  {
    HTE_it it = hashes.find(h);
    if(it == hashes.end()) return NULL;
    return it->second;
  }

  /* FIXED BUG: At this point, it's possible the 'file' passed to us
     is a reference to ent->file, which may get deleted by
     remove(file). So to avoid referencing deleted objects, this
     function takes value parameters rather than references.
   */
  void add(std::string file, Hash hash, time_t writeTime)
  {
    // Remove any existing entry first
    remove(file);

    Entry *ent = new Entry;

    ent->hash = hash;
    ent->file = file;
    ent->writeTime = writeTime;

    paths[file] = ent;
    hashes.insert(HTE_val(hash, ent));
  }

  // Get a HashToEntry iterator for a given file/hash pair. Asserts if
  // not found.
  HTE_it getHashIt(const std::string &file, const Hash &hash)
  {
    HTE_it hit;
    HTE_pair range = hashes.equal_range(hash);
    for(hit = range.first; hit != range.second; ++hit)
      if(hit->second->file == file)
        return hit;
    assert(0);
  }

  void remove(const std::string &file)
  {
    PTE_it it = paths.find(file);

    // Ignore missing entries
    if(it == paths.end()) return;

    // Get the entry and remove it
    Entry *ent = it->second;
    paths.erase(it);

    // Next remove the entry from the hash lookup
    HTE_it hit = getHashIt(file, ent->hash);
    hashes.erase(hit);

    // Kill the entry
    delete ent;
  }
};

CacheIndex::CacheIndex(const std::string &conf)
{
  ptr = new _CacheIndex_Hidden;
  if(conf != "") load(conf);
}
CacheIndex::~CacheIndex() { delete ptr; }

void CacheIndex::load(const std::string &conf)
{
  LOCK lock(ptr->mutex);
  ptr->loadConf(conf);
}

int CacheIndex::getStatus(const std::string &_where, const Hash &hash)
{
  PRINT("Cache::getStatus(" << _where << ", " << hash << ")");
  std::string where = abs(_where);
  PRINT("where=" << where);

  LOCK lock(ptr->mutex);

  // Check the index first if we think this is a match.
  Entry *ent = ptr->find(where);
  bool exists = bf::exists(where);
  bool checkAlt = false;
  if(!ent || ent->hash != hash || !exists)
    {
      /* If there probably isn't a match, then it's more efficient to
         check for alternatives first, rather than hashing a file we
         will potentially overwrite anyway. Alternatives (marked by
         CI_ElseWhere) always take precedence over mismatches
         (CI_Diff).
      */
      checkAlt = true;
      std::string alt = findHash(hash);
      if(alt != "")
        return CI_ElseWhere;
    }

  // If nothing was found, tell the user
  if(!exists)
    return CI_None;

  // Get real hash
  Hash real = addFile(where);

  // If it turns out it's a match after all, we're done.
  if(real == hash)
    return CI_Match;

  /* So the file exists and it doesn't match the requested hash. This
     means it's either CI_Diff or CI_Elsewhere. Do the alternatives
     check here if we didn't do it above.
   */
  if(!checkAlt)
    {
      std::string alt = findHash(hash);
      if(alt != "")
        return CI_ElseWhere;
    }

  // Just found a mismatch.
  return CI_Diff;
}

void CacheIndex::getEntries(CIVector &result) const
{
  LOCK lock(ptr->mutex);

  result.reserve(ptr->paths.size());

  PathToEntry::const_iterator it;
  for(it = ptr->paths.begin(); it != ptr->paths.end(); it++)
    {
      Entry &e = *(it->second);
      CIEntry e2 = { e.hash, e.file, e.writeTime };
      result.push_back(e2);
    }
}

std::string CacheIndex::findHash(const Hash &hash)
{
  PRINT("Cache::findHash(" << hash << ")");
  LOCK lock(ptr->mutex);

  Entry *ent = ptr->find(hash);
  while(ent)
    {
      std::string file = ent->file;

      PRINT("Found " << file);

      // Does the file still exist, and does it match?
      if(bf::exists(file) && addFile(file) == hash)
        return file;

      // That file didn't match after all. Try another one.
      Entry *ent2 = ptr->find(hash);

      /* If this still returned the same entry, we KNOW that this
         entry is invalid (since we just tested it above.) So remove
         it.

         This happens in several cases (non-existing file, non-
         matching file) and when the new entry added/replaced by
         addFile doesn't overwrite or remove the old one.
      */
      while(ent2 && ent2->file == file)
        {
          ptr->remove(file);
          ptr->removeConf(file);
          PRINT("Removed " << file);
          ent2 = ptr->find(hash);
        }
      ent = ent2;
    }
  return "";
}

void CacheIndex::removeFile(const std::string &_where)
{
  std::string where = abs(_where);
  LOCK lock(ptr->mutex);
  ptr->remove(where);
  ptr->removeConf(where);
}

/* This is the main 'workhorse' of the indexer, and the function that
   does most of the actual interaction with the filesystem.
 */
Hash CacheIndex::addFile(const std::string &_where, const Hash &given)
{
  assert(_where != "");
  std::string where = abs(_where);
  LOCK lock(ptr->mutex);

  // First things first: does the file even exist?
  if(!bf::exists(where))
    {
      // Make sure we haven't indexed it
      removeFile(where);

      // Then throw
      throw std::runtime_error("Cannot index non-existing file: " + where);
    }

  // Get file information
  uint64_t time = bf::last_write_time(where);
  uint64_t size = bf::file_size(where);

  // Check that the given hash, if any, isn't wrong.
  if(!given.isNull() && given.size() != size)
    throw std::runtime_error("Given hash doesn't match real file size: " + where);

  // Find the index entry
  Entry *ent = ptr->find(where);

  if(ent)
    {
      // The entry already exists. Check if it matches reality.
      bool match=true;
      if(ent->writeTime != time ||
         ent->hash.size() != size) match = false;
      if(!given.isNull() && given != ent->hash)
        match = false;

      if(match)
        // The existing entry looks right. We're done.
        return ent->hash;
    }

  /* Either there was no entry, or the entry didn't match
     reality. Create a new one. ptr->add() will kill any existing
     entry by the same name.
   */

  Hash hash = given;
  if(hash.isNull())
    // No hash provided. Hash the file ourselves.
    hash = HashStream::sum(where);
  assert(!hash.isNull());
  ptr->add(where, hash, time);
  ptr->addConf(where, hash, time);

  return hash;
}
