#ifndef __SPREAD_SR0_HPP_
#define __SPREAD_SR0_HPP_

/* SR0 (short for Spread-REST v0) is a prototype protocol to install
   or update a directory from data on a web server.

   Unlike the larger Spread system, it does not rely on a complex set
   of rules or pre-existing data. All it needs is an URL (or file
   path) to a repository, and it will bootstrap the rest.

   SpreadLib uses SR0 to fetch the complete ruleset and data from a
   source.

   SR0 itself uses the Spread rule system to update the directory, and
   can be much more bandwidth efficient than simply downloading a full
   zip. If set up correctly, knowledge of existing files is used to
   (sometimes greatly) reduce the amount of downloaded data needed.

   SR0 protocol:

   - the SR0 struct takes an input source, either an url or a file
     system directory, depending on whether you use fetchURL() or
     fetchFile()

   - here it expects to find two files:
     - /short.txt
     - /index.zip

   - the short.txt is expected to contain the start of the hash (at
     least 8 bytes of the standard base64url version) of the directory
     that represents the latest version. If this matches the installed
     directory, nothing is done.

   - the existing dir hash is read from /current.hash in the output
     directory, if it exists.

   - if short.txt does NOT match the installed dir (or if there is no
     existing dir/current.hash file), then index.zip is downloaded and
     unpacked to a temporary location.

   - inside index.zip we expect to find the following files:
     - packs.json - package file - should contain one package named "index"
     - rules.json - (optional) rules needed to install the dir

     The rules.json (if present) are loaded, and the package "index"
     is installed into the target directory using a normal Spread
     install.

     All files inside the index.zip archive are automatically hashed
     and will be used by the installer as needed. You should probably
     at least include the dir-object of the output directory somewhere
     in the zip, so that the installer knows what it is supposed to
     install.
 */

#include "job/jobinfo.hpp"
#include "install_system/jobmanager.hpp"

namespace Spread
{
  namespace SR0
  {
    /* Fetch the dir from 'url'.

       The function will check the latest version against a file
       called 'current.hash' in destDir, if it exists. If the dir is
       updated, this file will be updated as well. It's important that
       the data you are downloading does NOT contain any file called
       current.hash, or it will be overwritten!

       The supplied cache should contain any existing files you think
       might help in the process. It will also be updated to index all
       output files.

       If async==true, the install is run in a background thread.

       The wasUpdated bool, if present, is set (possibly from a
       working thread) to true if updated data was downloaded.
     */
    extern JobInfoPtr fetchURL(const std::string &url,
                               const std::string &destDir,
                               JobManagerPtr manager,
                               bool async=true,
                               bool *wasUpdated = NULL);

    /* Same as fetchURL, but fetch from a filesystem dir instead.

       This may still cause files to be downloaded if the installation
       data itself uses URL rules.
     */
    extern JobInfoPtr fetchFile(const std::string &dir,
                                const std::string &destDir,
                                JobManagerPtr manager,
                                bool async=true,
                                bool *wasUpdated = NULL);
  };
}
#endif
