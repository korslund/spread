#include "index.hpp"
#include <map>
#include <time.h>
#include <boost/filesystem.hpp>
#include "hash/hash_stream.hpp"
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

namespace bfs = boost::filesystem;

struct BoostFSystem : FSystem
{
  std::string abs(const std::string &file) { return bfs::absolute(file).string(); }
  bool exists(const std::string &file) { return bfs::exists(file); }
  uint64_t file_size(const std::string &file) { return bfs::file_size(file); }
  Hash hashSum(const std::string &file) { return HashStream::sum(file); }
  bool equivalent(const std::string &file1, const std::string &file2)
  { return bfs::equivalent(file1, file2); }
  uint64_t last_write_time(const std::string &file)
  { return bfs::last_write_time(file); }
};

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

typedef std::map<std::string,std::string> StrMap;

struct CacheIndex::_CacheIndex_Hidden
{
  PathToEntry paths;
  HashToEntry hashes;

  Misc::JConfig conf;
  boost::recursive_mutex mutex;

  std::string makeConf(const Hash &hash, time_t wtime)
  {
    char buf[16];
    snprintf(buf, 16, "%ld", wtime);
    return hash.toString() + " " + std::string(buf);
  }

  void addConf(const std::string &file, const Hash &hash, time_t wtime)
  {
    PRINT("addConf: file=" << file << " str=" << makeConf(hash, wtime));
    conf.set(file, makeConf(hash, wtime));
  }

  void addConf(const StrMap &entries,
               const StrSet &remove)
  {
    PRINT("addConf: entries=" << entries.size() << " remove=" << remove.size());
    conf.setMany(entries, remove);
  }

  void removeConf(const std::string &file)
  { conf.remove(file); }

  void loadConf(const std::string &file)
  {
    conf.load(file);

    // Insert into the list
    std::vector<std::string> names;
    names = conf.getNames();

    StrSet rem;
    for(int i=0; i<names.size(); i++)
      {
        const std::string &file = names[i];

        try
          {
            std::string val = conf.get(file);
            int split = val.find(' ');
            Hash hash(val.substr(0,split));
            time_t wtime = atoll(val.substr(split+1).c_str());

            if(file == "" || split == 0 || wtime == 0 || hash.isNull())
              // Remove broken entries
              rem.insert(file);
            else
              add(file, hash, wtime);
          }
        catch(...)
          {
            // Remove broken entries
            rem.insert(file);
          }
      }

    if(rem.size())
      addConf(StrMap(), rem);
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

  // Returns true if an entry was removed
  bool remove(const std::string &file)
  {
    PTE_it it = paths.find(file);

    // Ignore missing entries
    if(it == paths.end()) return false;

    // Get the entry and remove it
    Entry *ent = it->second;
    paths.erase(it);

    // Next remove the entry from the hash lookup
    HTE_it hit = getHashIt(file, ent->hash);
    hashes.erase(hit);

    // Kill the entry
    delete ent;

    return true;
  }
};

CacheIndex::CacheIndex(const std::string &conf, FSystem *_sys)
  : sys(_sys)
{
  ptr = new _CacheIndex_Hidden;
  if(conf != "") load(conf);
  if(!sys) sys = new BoostFSystem;
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
  std::string where = sys->abs(_where);
  PRINT("where=" << where);

  LOCK lock(ptr->mutex);

  // Check the index first if we think this is a match.
  Entry *ent = ptr->find(where);
  bool exists = sys->exists(where);
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
        {
          /* Sometimes the paths DO refer to the same file, even
             though the strings are different. Boost lets us check
             for this.
           */
          if(sys->equivalent(alt, where))
            return CI_Match;

          return CI_ElseWhere;
        }
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
        {
          if(sys->equivalent(alt, where))
            return CI_Match;
          return CI_ElseWhere;
        }
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
      if(sys->exists(file) && addFile(file) == hash)
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
  std::string where = sys->abs(_where);
  LOCK lock(ptr->mutex);
  ptr->remove(where);
  ptr->removeConf(where);
}

void CacheIndex::verify()
{
  CIVector vec;
  Hash::DirMap names;
  getEntries(vec);
  for(int i=0; i<vec.size(); i++)
    names[vec[i].file];
  checkMany(names);
}

void CacheIndex::checkMany(Hash::DirMap &files)
{
  PRINT("checkMany: " << files.size() << " entries");
  LOCK lock(ptr->mutex);

  // This is the list fed to the config file
  StrMap entries;
  StrSet rem;

  Hash::DirMap::iterator it;
  for(it = files.begin(); it != files.end(); it++)
    {
      std::string file = sys->abs(it->first);

      if(!sys->exists(file))
        {
          PRINT("checkMany: file NOT found: " << file);
          it->second = Hash();
          if(ptr->remove(file))
            {
              PRINT("  File found and removed from list");
              rem.insert(file);
            }
          continue;
        }

      PRINT("checkMany: file found - passing to addEntry");
      uint64_t time;
      Hash hash = addEntry(file, it->second, time);
      it->second = hash;
      if(time)
        entries[file] = ptr->makeConf(hash, time);
    }

  if(entries.size() || rem.size())
    ptr->addConf(entries, rem);
}

void CacheIndex::addMany(const Hash::DirMap &files,
                         const StrSet &remove)
{
  PRINT("addMany: " << files.size() << " entries, " << remove.size() << " to remove");
  LOCK lock(ptr->mutex);

  // This is the list fed to the config file
  StrMap entries;

  for(Hash::DirMap::const_iterator it = files.begin(); it != files.end(); it++)
    {
      std::string file = sys->abs(it->first);
      uint64_t time;
      Hash hash = addEntry(file, it->second, time);
      if(time)
        entries[file] = ptr->makeConf(hash, time);
    }
  for(StrSet::const_iterator it = remove.begin(); it != remove.end(); it++)
    {
      std::string file = sys->abs(*it);
      ptr->remove(file);
    }

  if(entries.size() || remove.size())
    ptr->addConf(entries, remove);
}

/* This is the main 'workhorse' of the indexer, and the function that
   does most of the actual interaction with the filesystem.

   Also returns the file time if the entry is to be added to
   config. If not, time is set to 0.
 */
Hash CacheIndex::addEntry(std::string &where, const Hash &given, uint64_t &time)
{
  PRINT("addEntry: where=" << where << "  given=" << given);

  assert(where != "");
  where = sys->abs(where);

  PRINT("  abs path=" << where);

  // First things first: does the file even exist?
  if(!sys->exists(where))
    {
      // Make sure we haven't indexed it
      removeFile(where);

      // Then throw
      throw std::runtime_error("Cannot index non-existing file: " + where);
    }

  PRINT("File exists, getting stats");

  // Get file information
  time = sys->last_write_time(where);
  uint64_t size = sys->file_size(where);

  PRINT("time=" << time << " size=" << size);

  // Check that the given hash, if any, isn't wrong.
  if(!given.isNull() && given.size() != size)
    throw std::runtime_error("Given hash doesn't match real file size: " + where);

  // Find the index entry, if any
  Entry *ent = ptr->find(where);
  if(ent)
    {
      PRINT("File entry found");

      // The entry already exists. Check if it matches reality.
      bool match=true;
      if(ent->writeTime != time ||
         ent->hash.size() != size) match = false;
      if(!given.isNull() && given != ent->hash)
        match = false;

      if(match)
        {
          // The existing entry looks right. We're done.
          time = 0;
          PRINT("Existing entry matches, keep it.");
          return ent->hash;
        }
    }

  /* Either there was no entry, or the entry didn't match
     reality. Create a new one. ptr->add() will kill any existing
     entry by the same name.
   */
  Hash hash = given;
  if(hash.isNull())
    {
      // No hash provided. Hash the file ourselves.
      PRINT("Hashing the file...");
      hash = sys->hashSum(where);
      PRINT("  Done.");
    }
  assert(!hash.isNull());
  PRINT("Adding to index");
  ptr->add(where, hash, time);
  PRINT("Done, returning hash=" << hash);
  return hash;
}

Hash CacheIndex::addFile(std::string where, const Hash &given, bool allowMissing)
{
  PRINT("addFile(" << where << ", " << given << ")");

  if(allowMissing && !sys->exists(where))
    return Hash();

  LOCK lock(ptr->mutex);
  uint64_t time;
  Hash hash = addEntry(where, given, time);
  if(time)
    {
      PRINT("Entry added, now adding to config.");
      ptr->addConf(where, hash, time);
    }
  return hash;
}
