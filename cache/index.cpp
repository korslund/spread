#include "index.hpp"
#include <map>
#include <time.h>
#include <boost/filesystem.hpp>
#include "hash/hash_stream.hpp"
#include <stdint.h>
namespace bf = boost::filesystem;

using namespace Cache;
using namespace Spread;

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

struct CacheIndex::_CacheIndex_Hidden
{
  PathToEntry paths;
  HashToEntry hashes;

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

  void add(const std::string &file, const Hash &hash, time_t writeTime)
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

CacheIndex::CacheIndex() { ptr = new _CacheIndex_Hidden; }
CacheIndex::~CacheIndex() { delete ptr; }

int CacheIndex::getStatus(const std::string &where, const Hash &hash)
{
  // Check the index first if we think this is a match.
  Entry *ent = ptr->find(where);
  bool exists = bf::exists(where);
  bool checkAlt = false;
  if(!ent || ent->hash != hash || !exists)
    {
      // If it probably isn't, then it's more efficient to check for
      // alternatives first, rather than hashing a file we will
      // overwrite anyway. Alternatives always take precedence over
      // mismatches.
      checkAlt = true;
      std::string alt = findHash(hash);
      if(alt != "")
        return CI_ElseWhere;
    }

  // Check there there is no file
  if(!exists)
    return CI_None;

  // Get real hash
  Hash real = addFile(where);

  // If it turns out it's a match after all, we're done.
  if(real == hash)
    return CI_Match;

  /* So the file exists and it doesn't match the requested hash. So
     it's either CI_Diff or CI_Elsewhere. Do the alternatives check
     here if we didn't do it above.
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

std::string CacheIndex::findHash(const Hash &hash)
{
  Entry *ent = ptr->find(hash);
  while(ent)
    {
      // Check the file before returning
      if(addFile(ent->file) == hash)
        return ent->file;

      // Oops, that file didn't match after all. Try another one.
      ent = ptr->find(hash);
    }
  return "";
}

void CacheIndex::removeFile(const std::string &where)
{ ptr->remove(where); }

/* This is the main 'workhorse' of the indexer, and the function that
   does most of the actual interaction with the filesystem.
 */
Hash CacheIndex::addFile(const std::string &where, const Hash &h)
{
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
  if(!h.isNull() && h.size() != size)
    throw std::runtime_error("Given hash doesn't match real file size: " + where);

  // Find the index entry
  Entry *ent = ptr->find(where);

  if(ent)
    {
      // The entry already exists. Check if it matches reality.
      bool match=true;
      if(ent->writeTime != time ||
         ent->hash.size() != size) match = false;
      if(!h.isNull() && h != ent->hash)
        match = false;

      if(match)
        // The existing entry looks right. We're done.
        return ent->hash;
    }

  /* Either there was no entry, or the entry didn't match
     reality. Create a new one. ptr->add() will kill any existing
     entry by the same name.
   */

  Hash hash = h;
  if(hash.isNull())
    // No hash provided. Hash the file ourself.
    hash = HashStream::sum(where);
  assert(!hash.isNull());
  ptr->add(where, hash, time);

  return hash;
}
