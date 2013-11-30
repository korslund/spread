#include "files.hpp"

#include <boost/filesystem.hpp>
#include <dir/from_fs.hpp>
#include <assert.h>

using namespace Cache;
using namespace Spread;
namespace bf = boost::filesystem;

std::string Files::makePath(const Hash &hash) const
{
  assert(hash.isSet());
  assert(basedir != "");
  std::string hstr = hash.toString();
  assert(hstr.size() > 2);
  bf::path res = basedir;
  res /= hstr.substr(0,2);
  res /= hstr.substr(2,std::string::npos);
  return res.string();
}

void Files::cacheAll() const
{
  assert(basedir != "");
  Hash::DirMap list;

  // fromFS() does exactly what we want - traverses the directory
  // recursively and caches all files in it.

  Dir::fromFS(list, basedir, index);
}

std::string Files::storePath(const Hash &hash) const
{
  std::string path = makePath(hash);
  bf::create_directories(bf::path(path).parent_path());

  Hash fhash = index.checkFile(path);

  if(fhash != hash)
    // Remove existing file if there is a mismatch
    bf::remove(path);
  else
    // Otherwise, return an empty string, signalling that this file is
    // already OK.
    path = "";

  return path;
}
