#include "listwriter.hpp"
#include <mangle/stream/clients/copy_stream.hpp>
#include <stdexcept>
#include <assert.h>
#include "unpack/dirwriter.hpp"
#include "hash/hash_stream.hpp"
#include "misc/readjson.hpp"

using namespace std;
using namespace Spread;
using namespace SpreadGen;
using namespace Mangle::VFS;
using namespace Mangle::Stream;

void ListWriter::write(const PackLister &lister, const string &where)
{
  StreamFactoryPtr ptr(new Unpack::DirWriter(where));
  write(lister, ptr);
}

void ListWriter::write(const PackLister &lst, StreamFactoryPtr output)
{
  // Packs list
  {
    Json::Value packs;

    map<string, PackLister::PackInfo>::const_iterator it;
    for(it = lst.packs.begin(); it != lst.packs.end(); it++)
      {
        Json::Value dirs, hints, total;

        for(int i=0; i<it->second.dirs.size(); i++)
          dirs.append(it->second.dirs[i].toString());

        for(int i=0; i<it->second.hints.size(); i++)
          hints.append(it->second.hints[i].toString());

        total.append(dirs);
        total.append(hints);

        packs[it->first] = total;
      }

    ReadJson::writeJson(output->open("packs.json"), packs);
  }

  // Rule file
  {
    Json::Value rules;

    {
      RuleList::const_iterator it;
      for(it = lst.ruleSet.begin(); it != lst.ruleSet.end(); it++)
        rules.append((*it)->ruleString);
    }
    {
      set<const ArcRuleData*>::const_iterator it;
      for(it = lst.arcSet.begin(); it != lst.arcSet.end(); it++)
        rules.append((*it)->ruleString);
    }

    ReadJson::writeJson(output->open("rules.json"), rules);
  }

  /* 1k-cache. This is a lookup of hash snippets for the first 1 Kb of
     the data block for each hash. This is useful to quickly validate
     URL rules - it lets you determine if a remote file has changed by
     just downloading the first Kb, instead of the entire file.

     Since this is only intended to validate URL rules, we only
     include entries need for that.

     We don't really use these yet, so don't bother implementing them
     yet.

  {
    Json::Value out1k;

    ReadJson::writeJson(output->open("1kcache.json"), out1k);
  }
   */

  // Dir files
  {
    PackLister::HashSet::const_iterator it;
    for(it = lst.dirs.begin(); it != lst.dirs.end(); it++)
      {
        const Hash &dirHash = *it;
        const string &dirFile = cache.index.findHash(dirHash);

        if(dirFile == "")
          throw runtime_error("Couldn't find any source for directory object: " + dirHash.toString());

        HashStreamPtr inp(new HashStream(dirFile));
        StreamPtr out = output->open("dirs/" + dirHash.toString());
        assert(out);

        CopyStream::copy(inp, out);

        if(inp->finish() != dirHash)
          throw runtime_error("Hash mismatch in file " + dirFile + " (expected " + dirHash.toString() + ")");
      }
  }
}
