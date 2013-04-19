#include "sr0.hpp"

#include <boost/filesystem.hpp>
#include <mangle/stream/servers/string_writer.hpp>
#include <mangle/stream/servers/file_stream.hpp>
#include <mangle/stream/servers/outfile_stream.hpp>
#include <mangle/stream/clients/copy_stream.hpp>
#include "htasks/unpackhash.hpp"
#include "job/thread.hpp"
#include "tasks/download.hpp"
#include "rules/rule_loader.hpp"
#include "misc/readjson.hpp"

//#define DEBUG_PRINT
#ifdef DEBUG_PRINT
#include <iostream>
#define PRINT(a) std::cout << __LINE__ << ": " << a << "\n"
#else
#define PRINT(a)
#endif

using namespace Spread;
using namespace Mangle::Stream;
using namespace boost::filesystem;

typedef Hash::DirMap HMap;

struct Sr0Job : Job
{
  std::string source;
  bool isUrl;
  path dest;

  Cache::Cache *cache;
  JobManagerPtr manager;

  bool *wasUpdated;

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

  // Load output version from a pack file
  Hash getPackVersion(const path &file)
  {
    if(!exists(file)) return Hash();
    Json::Value val = ReadJson::readJson(file.string());
    return Hash(val["index"]["dirs"][0].asString());
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
    PRINT("Starting SR0 job");
    setBusy("Checking for updates");
    assert(cache);
    assert(manager);

    if(wasUpdated) *wasUpdated = false;

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
            PRINT("Downloading " << fetch);
            DownloadTask dl(fetch, StringWriter::Open(netVer));
            if(runClient(dl)) return;
            PRINT("   Got: " << netVer);
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
            PRINT("No update necessary");
            setDone();
            return;
          }
      }

    setBusy("Fetching update info");
    PRINT("Fetching update info");

    // Next, get the the zip file
    std::string zipfile = source + "/index.zip";
    if(isUrl)
      {
        std::string url = zipfile;
        zipfile = cache->createTmpFilename();
        PRINT("Downloading " << url << " => " << zipfile);
        DownloadTask dl(url, zipfile);
        if(runClient(dl)) return;
        PRINT("  Done.");
      }
    else if(!exists(zipfile))
      {
        setError("File not found: " + zipfile);
        return;
      }

    // Unpack the archive into a temporary directory
    HMap index;
    path tmpOut = cache->createTmpFilename();
    PRINT("Unpacking " << zipfile << " => " << tmpOut);
    UnpackHash::makeIndex(zipfile, index, tmpOut.string());

    // Load the new version
    Hash newHash = getVersion(tmpOut/"index.hash");

    // If there was no index file, try loading the package "index"
    // from packs.json instead
    if(newHash.isNull())
      newHash = getPackVersion(tmpOut/"packs.json");

    if(newHash.isNull() || newHash == curHash ||
       (netVer != "" && !compVer(netVer, newHash)))
      {
        setError("Update version mismatch");
        return;
      }

    // Index all the extracted files
    PRINT("Adding files to cache index");
    for(HMap::iterator it = index.begin(); it != index.end(); it++)
      {
        // Don't use reference, because silly boost paths returns
        // a reference
        std::string file = (tmpOut/it->first).string();
        const Hash &hsh = it->second;
        PRINT(hsh << "  " << file);
        Hash res = cache->index.addFile(file, hsh);
        assert(res == hsh);
      }
    PRINT("  Done.");

    if(checkStatus()) return;

    // Load the ruleset file, if there is one.
    RuleSet rules;
    {
      std::string file = (tmpOut/"rules.json").string();
      if(exists(file))
        loadRulesJsonFile(rules, file);
    }

    InstallerPtr inst = manager->createInstaller(dest.string(), rules);

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
      inst->addHint(it->second);

    /* Add the requested output directory. Again, since we have
       indexed all the extracted files, the Installer should be able
       to load the dir contents automatically.
    */
    inst->addDir(newHash);

    // Run the install!
    JobInfoPtr inf = manager->addInst(inst);
    setBusy("Updating");
    PRINT("Running main installer => " << dest);
    //assert(inf->isInitiated()); // This may fail
    assert(manager->getInfo()->isInitiated()); // This is correct
    if(waitClient(inf)) return;
    setBusy("Cleaning up");
    PRINT("  Done.");

    // On success, write back the new version file
    OutFileStream out(hashFile);
    out.write(newHash.getData(), 40);

    // Notify the caller that we successfully updated the data, if
    // they requested it.
    if(wasUpdated) *wasUpdated = true;

    setDone();
  }
};

JobInfoPtr SR0::fetchFile(const std::string &dir, const std::string &destDir,
                          JobManagerPtr manager, bool async, bool *wasUpdated)
{
  Sr0Job *j = new Sr0Job;
  j->source = dir;
  j->isUrl = false;
  j->dest = destDir;
  j->cache = &manager->cache;
  j->manager = manager;
  j->wasUpdated = wasUpdated;
  return Thread::run(j,async);
}

JobInfoPtr SR0::fetchURL(const std::string &url, const std::string &destDir,
                         JobManagerPtr manager, bool async, bool *wasUpdated)
{
  PRINT("SR0: Fetching URL " << url << " => " << destDir);

  Sr0Job *j = new Sr0Job;
  j->source = url;
  j->isUrl = true;
  j->dest = destDir;
  j->cache = &manager->cache;
  j->manager = manager;
  j->wasUpdated = wasUpdated;
  return Thread::run(j,async);
}
