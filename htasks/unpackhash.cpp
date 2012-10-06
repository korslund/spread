#include "unpackhash.hpp"
#include "tasks/unpack.hpp"
#include <mangle/vfs/stream_factory.hpp>
#include <mangle/stream/servers/null_stream.hpp>
#include "hash/hash_stream.hpp"
#include <boost/filesystem.hpp>
#include <stdexcept>

using namespace Spread;
using namespace Mangle::Stream;

//#define DEBUG_PRINT
#ifdef DEBUG_PRINT
#include <iostream>
#define PRINT(a) std::cout << __LINE__ << ": " << a << "\n";
#else
#define PRINT(a)
#endif

/* This output writer is used for indexing. It stores names + hashes
   in an index, and can optionally write data to files.
 */

struct UH_ListHasher : Mangle::VFS::StreamFactory
{
  Hash::DirMap *index;

  HashStreamPtr stream;
  std::string lastName;
  bool lastDir;
  std::string where;
  bool absPaths;

  ~UH_ListHasher() { open(""); }

  // This follows the same close-then-open mechanics as
  // getOutStream()/closeStream() in HashTask.
  StreamPtr open(const std::string &name)
  {
    PRINT("open(" << name << ")  lastName=" << lastName);

    // Store the last hash, if any
    if(stream && lastName != "")
      {
        // Ignore directories
        if(!lastDir)
          (*index)[lastName] = stream->finish();
        lastName = "";
      }

    // Allow an empty name to close the last file (above) without
    // opening a new one here.
    if(name == "") return StreamPtr();

    // Is this a directory?
    char ch = name[name.size()-1];
    lastDir = (ch == '/' || ch == '\\');
    lastName = name;

    if(where == "")
      {
        // If we're not writing anything, just reuse a NullStream as
        // the final output.
        if(!stream)
          stream.reset(new HashStream(StreamPtr(new NullStream)));
      }
    else
      {
        using namespace boost::filesystem;
        path abs = where;
        abs /= name;

        PRINT("  abs=" << abs);

        if(absPaths)
          lastName = abs.string();

        stream.reset();
        if(!lastDir)
          {
            create_directories(abs.parent_path());
            stream.reset(new HashStream(abs.string(), true));
          }
        else
          {
            create_directories(abs);
          }
      }

    if(lastDir) return StreamPtr();
    return stream;
  }
};

void UnpackHash::makeIndex(const std::string &arcFile, Hash::DirMap &index,
                           const std::string &where)
{
  UH_ListHasher *m = new UH_ListHasher;
  m->index = &index;
  m->where = where;
  m->absPaths = false;
  Mangle::VFS::StreamFactoryPtr mp(m);

  UnpackTask unp(arcFile, mp);
  unp.run();
  // Needed to include the last file
  m->open("");

  unp.failError();
}

/* This output writer is used for unpacking. It looks up filenames and
   finds their hashes.
 */
struct UH_ListUser : Mangle::VFS::StreamFactory
{
  Hash::DirMap index;
  HashTask *owner;

  UH_ListUser(HashTask *t) : owner(t) { assert(owner); }

  StreamPtr open(const std::string &name)
  {
    // Look up the hash from the name
    Hash h = index[name];

    // Fail if the unpacker threw an unknown file at us
    if(h.isNull())
      throw std::runtime_error("Unexpected file in archive: " + name);

    // HashTask takes care of the rest
    return owner->getOutStream(h);
  }
};

Job *UnpackHash::createJob()
{
  // Get the input filename
  assert(inputs.size() == 1);
  std::string file = inputs.begin()->second;

  PRINT("UnpackHash::createJob: file=" << file);

  // Figure out if we are doing a "blind" unpack
  if(blindDir != "")
    {
      PRINT("  Blind unpack to: " << blindDir << " absPahts=" <<
            (absPaths?"true":"false"));

      UH_ListHasher *m = new UH_ListHasher;
      assert(blindOut);
      m->index = blindOut;
      m->where = blindDir;
      m->absPaths = absPaths;
      Mangle::VFS::StreamFactoryPtr mp(m);

      PRINT("Returning job");
      return new UnpackTask(file, mp);
    }

  // The rest is for non-blind unpacking

  UH_ListUser *m = new UH_ListUser(this);
  Mangle::VFS::StreamFactoryPtr mp(m);

  // Copy requested items from the full index over to the working
  // index
  Hash::DirMap::iterator it;

  // Just to optimize a tiny bit more - use this to prune repeated
  // hashes from the file list
  std::set<Hash> hashes;

  // Loop through the zip index and only pick the files we actually
  // want to use for unpacking.
  for(it = index.begin(); it != index.end(); ++it)
    {
      const std::string &name = it->first;
      const Hash &hsh = it->second;

      // Do we need this object?
      if(outputs.find(hsh) == outputs.end())
        // Hash was not found in the requested output list, skip it.
        continue;

      // Have we already listed this hash?
      if(hashes.count(hsh) != 0)
        // Prune duplicates
        continue;

      // Is the hash empty? Might be the case for directories. If so,
      // skip it.
      if(hsh.isNull())
        continue;

      /* This file is a keeper. Add it to the file list, as well as to
         the live lookup index.
       */
      list.insert(name);
      m->index[name] = hsh;
      hashes.insert(hsh);
    }

  // Set up the unpacking job
  return new UnpackTask(file, mp, &list);
}
