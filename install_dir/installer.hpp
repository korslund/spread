#ifndef __SPREAD_I_INSTALLER_HPP__
#define __SPREAD_I_INSTALLER_HPP__

#include <hash/hash.hpp>
#include <boost/shared_ptr.hpp>

namespace Spread
{
  struct Installer
  {
    /* Add a single file to the install list. The file path is taken
       to be relative to 'basedir'.

       If the file already exists and does not match a file added with
       remFile/remDir, the job will ask through owner.askWait()
       whether the user wants to overwrite the file.
    */
    virtual void addFile(const std::string &file, const Hash &hash) = 0;

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
    virtual void remFile(const std::string &file, const Hash &hash) = 0;

    // Convenience functions:
    virtual void updateFile(const std::string &file, const Hash &oldH, const Hash &newH)
    { addFile(file, newH); remFile(file, oldH); }
    virtual void ignoreFile(const std::string &file)
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
    virtual void addDir(const Hash::DirMap &dir, const std::string &path = "") = 0;
    virtual void remDir(const Hash::DirMap &dir, const std::string &path = "") = 0;

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
    virtual void addDir(const Hash &hash, const std::string &path = "") = 0;
    virtual void remDir(const Hash &hash, const std::string &path = "") = 0;

    /* Add a hint to the file-fetching system. A hint is a directory
       (or corresponding archive) hash that should be included in the
       file lookup process.

       Hints do not affect the list output files, only where we might
       FIND those output files.
     */
    virtual void addHint(const Hash &hint) = 0;
  };

  typedef boost::shared_ptr<Installer> InstallerPtr;
}
#endif
