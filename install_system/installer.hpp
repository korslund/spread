#ifndef __SPREAD_INSTALLER_HPP
#define __SPREAD_INSTALLER_HPP

#include "cache/cache.hpp"
#include "rules/ruleset.hpp"
#include "job/job.hpp"

/* Main installer job.

   Add dependencies (output files) through addDep() and the addDir()
   variants, then run the job like normal.
 */

namespace Spread
{
  struct Installer : Job
  {
    Installer(Cache::Cache &_cache, RuleSet &_rules,
              const std::string &_prefix = "");

    /* Add a single file to the install list. The file path is taken
       to be relative to 'prefix'.

       If the file already exists and does not match a file added with
       remFile/remDir, then the execution may return a recoverable
       error, asking the user whether to keep or overwrite the file.
    */
    void addFile(const std::string &file, const Hash &hash);

    // Convenience functions
    void addFile(const std::string &file, const Hash &hash, const Hash &existing)
    { addFile(file, hash); remFile(file, existing); }
    void ignoreFile(const std::string &file, const Hash &hash)
    { addFile(file, hash, hash); }

    /* List a single file as already existing in the output directory.
       This has several uses:

       - if the file is both added with addFile() and listed with
         remFile() with the SAME hash, then the file IGNORED in the
         install process (it's irrelevant whether or not any physical
         file matches the given hash.) Use this to mark eg. initial
         config files when updating, so that the update doesn't
         inadvertently overwrite user changes.

       - if remFile() is called and NO matching file is listed with
         addFile(), then the file is DELETED.

       - if a file is deleted in one places and added in another, the
         installer will usually just rename/move the file.

       The net effect is that if remFile/remDir is called with the
       known _existing_ contents of a directory, and addFile/addDir
       with the _new_ (updated) contents of a directory, then the
       installer will replace or patch files as expected, add and
       delete files as required, and leave all unchanged files alone
       (even if they have user modifications.)

       The function is not necessary for patching. If patching an
       existing file (from the same location or not) can be used to
       create the destination file, then patching is used regardless
       of whether remFile() was called.
     */
    void remFile(const std::string &file, const Hash &hash);

    /* Add a directory of dependencies. All paths are relative to
       'prefix'. You can also specify an optional path to prepend to
       all file paths. The path MUST BE SLASH TERMINATED if you want
       it to act as a directory. The final path becomes:

       prefix + path + path_in_directory

       The given hash may either refer to a directory object, or to an
       archive hash (in which case the entire archive is added as-is.)

       The function REQUIRES that the directory is either available as
       a loadable object, or that there is a matching archive rule for
       the directory.

       The installer will ask the RuleSet for any additional hints for
       finding the directory object, required archive file(s), or any
       other rule that may help the installation.
     */
    void addDir(const Hash &hash, const std::string &path = "");

    /* The remFile()-based equivalent of remDir().

       Notes: Like addDir() this asks the rule system for extra hints,
       but unlike addDir() the hints are used for PATCHING the listed
       files (if possible), not for installing them.
     */
    void remDir(const Hash &hash, const std::string &path = "");

  private:
    struct _Internal;
    boost::shared_ptr<_Internal> ptr;
  };
}

#endif
