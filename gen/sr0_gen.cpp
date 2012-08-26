#include "sr0_gen.hpp"
#include "listwriter.hpp"
#include <mangle/stream/servers/outfile_stream.hpp>
#include <boost/filesystem.hpp>

using namespace SpreadGen;
using namespace Spread;

void GenSR0::makeSR0(const std::string &arcFile, const std::string &outDir)
{ makeSR0(cache.index.addFile(arcFile), outDir); }

void GenSR0::makeSR0(const Hash &hash, const std::string &outDir)
{
  boost::filesystem::path out = outDir;

  PackLister lst(cache, rules);
  Hash dir = lst.addDir("index", hash);

  ListWriter wrt(cache);
  wrt.write(lst, (out / "_output").string());

  // Write first 8 bytes of hash to 'short.txt'
  Mangle::Stream::OutFileStream::Write
    ((out / "short.txt").string(), dir.toString().substr(0,8));
}
