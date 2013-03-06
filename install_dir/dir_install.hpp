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
                 Cache::ICacheIndex &cache, const std::string &prefix);

    /* Add a single file to the install list. The file path is taken
       to be relative to 'prefix'.

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

       Note that calling remFile() is not necessary to invoke the
       backend patching mechanism. If patching an existing file (from
       the same location or not) can be used to create the destination
       file, then patching is used regardless of whether remFile() was
       called.
     */
    void remFile(const std::string &file, const Hash &hash);

    // Convenience functions:

    void updateFile(const std::string &file, const Hash &hash, const Hash &existing)
    { addFile(file, hash); remFile(file, existing); }

    void ignoreFile(const std::string &file, const Hash &hash)
    { updateFile(file, hash, hash); }

    /* Add/remove a directory of dependencies. All paths are relative
       to 'prefix'. You can also specify an optional path to prepend
       to all file paths. The path MUST BE SLASH TERMINATED if you
       want it to act as a directory. The final path becomes:

       prefix + path + path_in_directory

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
       finding the directory object, required archive file(s), or any
       other rule that may help the process.
     */
    void addDir(const Hash &hash, const std::string &path = "");
    void remDir(const Hash &hash, const std::string &path = "");

    void doJob();

  private:
    struct _Internal;
    boost::shared_ptr<_Internal> ptr;

  protected:
    // Internal functions:
    void loadHints(const Hash &dirHash);
    DirPtr addDirFile(Hash::DirMap &out, const Hash &dirHash,
                      const std::string &path);
    void handleHash(Hash::DirMap &out, const Hash &dirHash,
                    HashDir &blinds, const std::string &path);
    void sortInput();
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
  };
}

#endif
