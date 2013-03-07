#ifndef __SPREAD_DIR_INSTALLER_HPP__
#define __SPREAD_DIR_INSTALLER_HPP__

#include <install_jobs/treebase.hpp>
#include <dir/ptr.hpp>
#include <rules/ruleset.hpp>
#include <cache/iindex.hpp>
#include <parent_job/userask.hpp>

namespace Spread
{
  struct DirOwner;
  struct DirInstaller : TreeBase
  {
    DirInstaller(DirOwner &owner, RuleSet &rules,
                 Cache::ICacheIndex &_cache, const std::string &basedir);

    /* Add a single file to the install list. The file path is taken
       to be relative to 'basedir'.

       If the file already exists and does not match a file added with
       remFile/remDir, the job will ask through owner.askWait()
       whether the user wants to overwrite the file.
    */
    void addFile(const std::string &file, const Hash &hash);

    /* List a single file as already existing in the output directory.
       This has several uses:

       - if the file is both added with addFile() and remFile() with
         the SAME hash, then the file IGNORED in the install process
         (it's irrelevant whether or not any physical file matches the
         given hash - the location is not inspected.)

         Rationale: Files that are not scheduled to change in an
         update should not be touched, since that might inadvertently
         overwrite user changes.

       - if remFile() is called and NO matching file is listed with
         addFile(), then the file is DELETED. However if it does not
         match the requested hash, the user is asked first through
         owner.askWait().

         Rationale: Files removed in an output should be deleted, but
         user-modified files should not be removed without
         confirmation.

       - if a file is deleted in one places and added in another, the
         installer will usually rename/move the file as an
         optimization.

       The net effect is that if remDir is called with the known
       _existing_ contents of a directory, and addDir with the _new_
       (updated) contents of a directory, then the installer will
       replace or patch files as expected, create or delete files as
       required, and leave all unchanged files alone (even if they
       contain user modifications.) All affected files with user
       modifications cause a recoverable error.
     */
    void remFile(const std::string &file, const Hash &hash);

    // Convenience functions:

    void updateFile(const std::string &file, const Hash &oldH, const Hash &newH)
    { addFile(file, newH); remFile(file, oldH); }

    void ignoreFile(const std::string &file)
    { Hash h("IGNORE"); updateFile(file, h, h); }

    /* Add/remove a directory of dependencies. All paths are relative
       to 'basedir'. You can also specify an optional path to prepend
       to all file paths. Paths will automatically be slash
       terminated. The result will be:

       prefix / path / path_in_directory

       These functions are equivalent of calling addFile()/remFile()
       multiple times in a row. If you want more functionality, see
       the hash version below.
    */
    void addDir(const Hash::DirMap &dir, const std::string &path = "");
    void remDir(const Hash::DirMap &dir, const std::string &path = "");

    /* 'Enhanced' hash versions of addDir()/remDir().

       The given hash may either refer to a directory object, or to an
       archive hash (in which case a matching archive rule must exist
       containing the dirhash.)

       In either case, if the directory matches a known archive
       dirhash, and the archive is available, it will be installed
       directly into the output directory. A pre-existing dir object
       is not required.

       Otherwise, the function REQUIRES that the directory is
       available as a binary loadable object somewhere (in the rule
       system or cache.)

       The installer may ask the RuleSet for any additional hints for
       finding the directory object, possibly fetching additional
       files and rules in the process.
     */
    void addDir(const Hash &hash, const std::string &path = "");
    void remDir(const Hash &hash, const std::string &path = "");

    void doJob();

  private:
    struct _Internal;
    boost::shared_ptr<_Internal> ptr;

  protected:
    std::string prefix;
    Cache::ICacheIndex &index;

    Hash::DirMap pre, post;
    HashDir preHash, postHash, preBlinds, postBlinds;

    typedef std::map<std::string, std::string> StrMap;

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

    // Delete a file from the filesystem
    virtual void deleteFile(const std::string &path) = 0;

    /* Move a file. This is required to work even if the underlying OS
       'move' function fails (eg. if trying to move across disk
       partitions.) On failure the function must fallback to vanilla
       copy+delete.
     */
    virtual void moveFile(const std::string &from, const std::string &to) = 0;
  };
}

#endif
