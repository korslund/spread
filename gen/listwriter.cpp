#include "listwriter.hpp"
#include <mangle/stream/clients/copy_stream.hpp>
#include <stdexcept>
#include <assert.h>
#include "unpack/dirwriter.hpp"
#include "hash/hash_stream.hpp"
#include "spreadlib/infojson.hpp"
#include "rule_writer.hpp"
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

// Could move this to spreadlib/infojson.cpp later, but it's not needed ATM.
static void writePackListJson(const PackInfoList &list, Mangle::Stream::StreamPtr strm)
{
  Json::Value packs;
  for(int i=0; i<list.size(); i++)
    {
      const PackInfo &pinf = list[i];

      if(pinf.dirs.size() == 0)
        throw runtime_error("Package '" + pinf.package + "' has no output directories.");

      packs[pinf.package] = infoToJson(pinf);
    }

  ReadJson::writeJson(strm, packs);
}

void ListWriter::write(const PackLister &lst, StreamFactoryPtr output)
{
  // Packs list
  {
    // Convert to PackInfoList first.
    PackInfoList out;
    out.reserve(lst.packs.size());
    map<string, PackInfo>::const_iterator it;
    for(it = lst.packs.begin(); it != lst.packs.end(); it++)
      {
        assert(it->first == it->second.package);
        out.push_back(it->second);
      }

    // Write the list to file
    writePackListJson(out, output->open("packs.json"));
  }

  // Rule file
  writeRulesJson(lst.ruleSet, lst.arcSet, output->open("rules.json"));

  // Dir files
  {
    PackLister::HashSet::const_iterator it;
    for(it = lst.dirs.begin(); it != lst.dirs.end(); it++)
      {
        const Hash &dirHash = *it;
        const string &dirFile = cache.findHash(dirHash);

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
