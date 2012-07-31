#ifndef __SPREAD_INSTALLER_HPP
#define __SPREAD_INSTALLER_HPP

#include "action_installer.hpp"
#include "cache/cache.hpp"
#include "rules/ruleset.hpp"

/* Main installer.

   Add dependencies (output files) through addDep and the addDir()
   variants, then run the installation with ActionInstaller::start().
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

    // Add a directory of dependencies. All paths are relative to
    // 'prefix'.
    void addDir(const Directory *dir);
    void addDir(DirectoryCPtr dir);

    /* Add a directory by hash. This assumes the directory object is
       available through Cache::loadDir().

       If alsoAsHint==true (default), then any archive rule that
       contains this directory is also automatically loaded.
     */
    void addDir(const Hash &hash, bool alsoAsHint = true);

  private:
    // Implemented from ActionInstaller:
    std::string brokenURL(const Hash &hash, const std::string &url);
    void getActions(ActionMap &acts);
    std::string getTmpFile(const Hash &h);
    void addToCache(const Hash &h, const std::string &file);

    Cache::Cache &cache;
    RuleSet &rules;
    boost::shared_ptr<ActionBuilder> build;
  };
}

#endif
