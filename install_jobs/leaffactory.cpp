#include "leaffactory.hpp"

#include <boost/filesystem.hpp>

#include <htasks/copyhash.hpp>
#include <htasks/downloadhash.hpp>
#include <htasks/unpackhash.hpp>

using namespace Spread;
namespace bf = boost::filesystem;

enum Types
  {
    T_Copy,
    T_Download,
    T_Unpack,
    T_UnpackBlind
  };

struct Target : TreeBase
{
  HashDir ins, outs;
  int type;
  std::string value;
  Hash dirHash;

  Target(TreeOwner &o, const std::string &val, int tp, const Hash &dh = Hash())
    : TreeBase(o), type(tp), value(val), dirHash(dh)
  {}

  void addOutput(const Hash &h, const std::string &where)
  { outs.insert(HDValue(h,where)); }
  void addInput(const Hash &h)
  { ins.insert(HDValue(h,"")); }

  void doJob()
  {
    assert(finder);
    Hash::DirMap arcdir;

  restart:

    HashTaskBase *task = NULL;
    if(type == T_Copy)
      {
        assert(outs.size() != 0);
        assert(ins.size() == 0);
        assert(value != "");
        setStatus("Copying file " + value);
        task = new CopyHash(value);
      }
    else if(type == T_Download)
      {
        assert(outs.size() != 0);
        assert(ins.size() == 0);
        assert(value != "");
        setStatus("Downloading URL " + value);
        task = new DownloadHash(value);
      }
    else if(type == T_Unpack)
      {
        assert(outs.size() != 0);
        assert(ins.size() == 1);
        assert(value == "");
        assert(!dirHash.isNull());

        setStatus("Unpacking dirHash " + dirHash.toString());

        // Fetch and load the dir file
        HashDir tmp;
        HashMap res;
        tmp.insert(HDValue(dirHash,""));
        fetchFiles(tmp, res);
        const std::string &file = res[dirHash];
        owner.loadDir(file, arcdir, dirHash);
        task = new UnpackHash(arcdir);
      }
    else if(type == T_UnpackBlind)
      {
        /* "Blind" unpacks are when we want the entire archive
           unpacked into a directory. This special case does not
           require knowing the directory up front, so we do away with
           finding and loading the dirfile.

           It does however mean we do not know any of the archive
           member hashes up front. Therefore the normal method of
           adding task outputs (in 'outs') does not work, because
           there's no way to know which file in the archive matches
           which hash.

           Instead we simply dump the archive contents to a directory,
           and generate the hash information on the fly. If no
           directory is given, the archive is just indexed (meaning
           all files are hashed in memory and a dir is created.)
         */
        assert(ins.size() == 1);
        assert(outs.size() == 0);
        if(value != "")
          setStatus("Blind unpacking into " + value);
        else
          setStatus("Blind indexing archive");
        task = new UnpackHash(value, arcdir);
      }
    else assert(0);
    assert(task);

    if(ins.size())
      {
        log("Fetching input file(s)");
        HashMap res;
        fetchFiles(ins, res);
        HashMap::const_iterator it;
        for(it = res.begin(); it != res.end(); it++)
          {
            const Hash &hash = it->first;
            const std::string &file = it->second;
            task->addInput(hash, file);
          }
      }

    Hash::DirMap dir;
    HashDir::const_iterator it;
    for(it = outs.begin(); it != outs.end(); it++)
      {
        const Hash &hash = it->first;
        std::string name = it->second;

        if(name == "")
          name = owner.getTmpName(hash);
        dir[name] = hash;
        task->addOutput(hash, name);
      }

    if(type == T_Unpack || type == T_UnpackBlind) log("Starting unpack");
    if(!execJob(task, type != T_Download))
      {
        // Allow failure recovery on URL errors
        assert(type == T_Download);
        assert(outs.size() != 0);
        assert(lastJob->isNonSuccess());

        // Only retry if the job failed, not if it was aborted
        if(lastJob->isError())
          {
            const Hash &hash = outs.begin()->first;

            // Report the broken URL, and try getting another one
            finder->brokenURL(hash, value);
            HashSource src;
            finder->findHash(hash, src);
            if(src.type == TST_Download && src.value != value)
              {
                // A new URL was found, try it.
                value = src.value;
                assert(value != "");
                goto restart;
              }
          }

        // If we have been aborted ourselves, then that overrides
        // error messages (otherwise we will return an error status on
        // aborts, when we should return abort status.)
        if(checkStatus()) return;
        lastJob->failError();
      }

    if(type == T_UnpackBlind)
      {
        // Cache the generated dir file
        owner.storeDir(arcdir, dirHash);

        // Add all the generated files to the 'dir' output.
        if(value != "")
          {
            bf::path base(value);
            for(Hash::DirMap::const_iterator it = arcdir.begin();
                it != arcdir.end(); ++it)
              dir[(base/it->first).string()] = it->second;
          }
      }

    if(dir.size())
      {
        finder->addToCache(dir);
        owner.notifyFiles(dir);
      }

    setDone();
  }
};

TreePtr LeafFactory::copyTarget(TreeOwner &owner, const std::string &from)
{ return TreePtr(new Target(owner, from, T_Copy)); }

TreePtr LeafFactory::downloadTarget(TreeOwner &owner, const std::string &url)
{ return TreePtr(new Target(owner, url, T_Download)); }

TreePtr LeafFactory::unpackTarget(TreeOwner &owner, const Hash &dir)
{ return TreePtr(new Target(owner, "", T_Unpack, dir)); }

TreePtr LeafFactory::unpackBlindTarget(TreeOwner &owner, const std::string &where,
                                       const Hash &dir)
{ return TreePtr(new Target(owner, where, T_UnpackBlind, dir)); }
