#include "files.hpp"

#include <boost/filesystem.hpp>

using namespace Cache;
using namespace Spread;
namespace bf = boost::filesystem;

std::string Files::makePath(const Hash &hash) const
{
  assert(hash.isSet());
  std::string hstr = hash.toString();
  assert(hstr.size() > 2);
  bf::path res = basedir;
  res /= hstr.substr(0,2);
  res /= hstr.substr(2,std::string::npos);
  return res.string();
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
