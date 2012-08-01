#include "sr0.hpp"

#include <boost/filesystem.hpp>
#include <mangle/stream/servers/string_writer.hpp>
#include <mangle/stream/servers/file_stream.hpp>
#include <mangle/stream/servers/outfile_stream.hpp>
#include <mangle/stream/clients/copy_stream.hpp>
#include "tasks/curl.hpp"
#include "htasks/unpackhash.hpp"
#include "job/thread.hpp"
#include "install/installer.hpp"
#include "tasks/download.hpp"
#include "rules/rule_loader.hpp"

using namespace Spread;
using namespace Mangle::Stream;
using namespace boost::filesystem;
using namespace Jobify;

typedef UnpackHash::HashMap HMap;

#include <iostream>
using namespace std;

struct Sr0Job : Job
{
  std::string source;
  bool isUrl;
  path dest;

  Cache::Cache *cache;

  // Load current version file
  Hash getVersion(const path &file)
  {
    if(!exists(file)) return Hash();

    uint8_t data[40];

    try
      {
        FileStream inf(file.string());
        if(inf.read(data, 40) != 40)
          return Hash();
      }
    catch(...) { return Hash(); }
    Hash h;
    h.copy(data);
    return h;
  }

  // Check a short version string against a hash. True if matches.
  bool compVer(const std::string &shortV,
               const Hash &hash)
  {
    // The short version doesn't need to contain the entire hash, just
    // the start of it.
    std::string longV = hash.toString();
    return (shortV.size() >= 8 &&
            shortV.size() <= longV.size() &&
            shortV == longV.substr(0,shortV.size()));
  }

  void doJob()
  {
    setBusy("Checking for updates");
    assert(cache);

    // Load current version indicator, if it exists
    std::string hashFile = (dest/"current.hash").string();
    Hash curHash = getVersion(hashFile);

    // Compare to latest version
    std::string netVer;
    if(!curHash.isNull())
      {
        std::string fetch = source + "/short.txt";

        if(isUrl)
          {
            cURL::get(fetch, StringWriter::Open(netVer),
                      Tasks::DownloadTask::userAgent);
          }
        else
          {
            CopyStream::copy(FileStream::Open(fetch),
                             StringWriter::Open(netVer));
          }

        // It it a match?
        if(compVer(netVer, curHash))
          {
            // Nothing to do, exit.
            setDone();
            return;
          }
      }
    if(checkStatus()) return;

    setBusy("Fetching update info");

    // Next, get the the zip file
    std::string zipfile = source + "/index.zip";
    if(isUrl)
      {
        std::string url = zipfile;
        zipfile = cache->createTmpFilename();
        cURL::get(url, zipfile, Tasks::DownloadTask::userAgent);
      }
    else if(!exists(zipfile))
      {
        setError("File not found: " + zipfile);
        return;
      }

    if(checkStatus()) return;

    // Unpack the archive into a temporary directory
    HMap index;
    path tmpOut = cache->createTmpFilename();
    UnpackHash::makeIndex(zipfile, index, tmpOut.string());

    // Load the new version
    Hash newHash = getVersion(tmpOut/"index.hash");

    if(newHash == curHash ||
       (netVer != "" && !compVer(netVer, newHash)))
      {
        setError("Update version mismatch");
        return;
      }

    // Index all the extracted files
    for(HMap::iterator it = index.begin(); it != index.end(); it++)
      {
        Hash h = cache->index.addFile((tmpOut/it->first).string(), it->second);
        assert(h == it->second);
      }

    if(checkStatus()) return;

    // Load the ruleset file, if there is one.
    RuleSet rules;
    {
      std::string file = (tmpOut/"rules.json").string();
      if(exists(file))
        loadRulesJsonFile(rules, file);
    }

    Installer inst(*cache, rules, dest.string());

    /* Add all the hashes in the original zip as archive hints to the
       installer. Since the zip file includes all the necessary dir
       files, some of the hashes in the zip will automatically match
       dirs associated with archives as well. Since we have indexed
       all the files, the Installer should also be able to load all
       the dir objects directly.

       So this essentially preloads all our archives rules. Hashes in
       the zip that do NOT match anything meaningful are just ignored
       by addHint().
     */
    for(HMap::iterator it = index.begin(); it != index.end(); it++)
      inst.addHint(it->second);

    /* Add the requested output directory. Again, since we have
       indexed all the extracted files, the Installer should be able
       to load the dir contents automatically.
    */
    inst.addDir(newHash);

    if(checkStatus()) return;

    // Run the install!
    setBusy("Updating");
    JobInfoPtr inf = inst.start(false);

    if(inf->isNonSuccess())
      {
        setError(inf->message);
        return;
      }

    // On success, write back the new version file
    OutFileStream out(hashFile);
    out.write(newHash.getData(), 40);

    setDone();
  }
};

JobInfoPtr SR0::fetchFile(const std::string &dir, const std::string &destDir,
                          Cache::Cache &cache, bool async)
{
  Sr0Job *j = new Sr0Job;
  j->source = dir;
  j->isUrl = false;
  j->dest = destDir;
  j->cache = &cache;
  JobInfoPtr info = j->getInfo();
  Thread::run(j,async);
  return info;
}

JobInfoPtr SR0::fetchURL(const std::string &url, const std::string &destDir,
                         Cache::Cache &cache, bool async)
{
  Sr0Job *j = new Sr0Job;
  j->source = url;
  j->isUrl = true;
  j->dest = destDir;
  j->cache = &cache;
  JobInfoPtr info = j->getInfo();
  Thread::run(j,async);
  return info;
}
