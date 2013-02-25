#ifndef __SPREAD_INSTALLER_HPP
#define __SPREAD_INSTALLER_HPP

#include "action_installer.hpp"
#include "cache/cache.hpp"
#include "rules/ruleset.hpp"

/* Main installer job.

   Add dependencies (output files) through addDep and the addDir()
   variants, then run the job like normal.
 */

namespace Spread
{
  struct ActionBuilder;
  struct Installer : ActionInstaller
  {
    Installer(Cache::Cache &_cache, RuleSet &_rules,
              const std::string &_prefix = "");

    /* Add archive hint. The hash can be either for the archive file
       itself or for the directory of its contents.

       Archive hints are used to preload archive directories so we can
       find the files inside. If you do not 'prime' the installer with
       the necessary archives before installation, no archives will be
       used.
    */
    void addHint(const Hash &hint);

    // Add a single dependency. The file path is taken to be relative
    // to 'prefix'.
    void addDep(const std::string &file, const Hash &hash);

    /* Add a directory of dependencies. All paths are relative to
       'prefix'. You can also specify an optional path to prepend to
       all file paths. The path must be slash terminated if you want
       it to become a directory! The final path becomes:

       prefix + path + path_in_directory
    */
    void addDir(const Directory *dir, const std::string &path = "");
    void addDir(DirectoryCPtr dir, const std::string &path = "");

    /* Add a directory by hash. This assumes that either the directory
       is available as a loadable cached object somewhere, or that
       there is a matching archive rule for the directory.

       If alsoAsHint==true (default), then any archive rule that
       contains this directory is loaded, if it exists.

       If the directory object does not exist, but an archive rule is
       found, the directory is added as a "blind" unpack. That means
       that whatever the archive contains is used as the output
       directory.
     */
    void addDir(const Hash &hash, bool alsoAsHint = true,
                const std::string &path = "");

  private:
    // Implemented from ActionInstaller:
    std::string brokenURL(const Hash &hash, const std::string &url);
    void getActions(ActionMap &acts);
    std::string getTmpFile(const Hash &h);
    void addToCache(const Hash::DirMap &list);

    Cache::Cache &cache;
    RuleSet &rules;
    boost::shared_ptr<ActionBuilder> build;
  };
}

#endif
