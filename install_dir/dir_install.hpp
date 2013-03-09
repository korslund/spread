#ifndef __SPREAD_DIR_INSTALLER_HPP__
#define __SPREAD_DIR_INSTALLER_HPP__

#include "installer.hpp"
#include <install_jobs/treebase.hpp>
#include <dir/ptr.hpp>
#include <rules/ruleset.hpp>
#include <cache/iindex.hpp>
#include <parent_job/userask.hpp>

namespace Spread
{
  struct DirOwner;
  struct DirInstaller : TreeBase, Installer
  {
    DirInstaller(DirOwner &owner, RuleSet &rules,
                 Cache::ICacheIndex &_cache, const std::string &basedir);

    // These are documented in installer.hpp
    void addFile(const std::string &file, const Hash &hash);
    void remFile(const std::string &file, const Hash &hash);
    void addDir(const Hash::DirMap &dir, const std::string &path = "");
    void remDir(const Hash::DirMap &dir, const std::string &path = "");
    void addDir(const Hash &hash, const std::string &path = "");
    void remDir(const Hash &hash, const std::string &path = "");

  private:
    struct _Internal;
    boost::shared_ptr<_Internal> ptr;

  protected:
    std::string prefix;
    Cache::ICacheIndex &index;

    Hash::DirMap pre, post;
    HashDir preHash, postHash, preBlinds, postBlinds;

    typedef std::map<std::string, std::string> StrMap;

    void doJob();

    // Internal functions:
    void loadHints(const Hash &dirHash);
    DirPtr addDirFile(Hash::DirMap &out, const Hash &dirHash,
                      const std::string &path);
    void handleHash(Hash::DirMap &out, const Hash &dirHash,
                    HashDir &blinds, const std::string &path);
    void sortInput();
    void sortBlinds();
    void sortAddDel(HashDir &add, HashDir &del, Hash::DirMap &upgrade);
    void resolveConflicts(HashDir &add, HashDir &del, const Hash::DirMap &upgrade);
    void findMoves(HashDir &add, HashDir &del, StrMap &moves);
    void doMovesDeletes(const StrMap &moves, const HashDir &del);
    int ask(const std::string &question, const std::string &opt0,
            const std::string &opt1 = "", const std::string &opt2 = "",
            const std::string &opt3 = "", const std::string &opt4 = "");
  };

  struct DirOwner : TreeOwner
  {
    /*
      Ask the main thread a question. If it returns true, it means the
      asker should abort. Otherwise, the response is stored in the
      UserAsk object.

      Pass along your own JobInfoPtr to make sure external abort
      requests are handled while waiting for the answer.
    */
    virtual bool askWait(AskPtr ask, JobInfoPtr info) = 0;

    /* Delete a file from the filesystem.
     */
    virtual void deleteFile(const std::string &path) = 0;

    /* Move a file. This is required to work even if the underlying OS
       'move' function fails (eg. if trying to move across disk
       partitions.) On failure the function must fallback to vanilla
       copy+delete.

       The created file is added to cache automatically.
     */
    virtual void moveFile(const std::string &from, const std::string &to) = 0;
  };
}
#endif
