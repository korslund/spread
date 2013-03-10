#include "from_fs.hpp"

#include <boost/filesystem.hpp>
#include <assert.h>

using namespace Spread;
namespace bf = boost::filesystem;

static std::string addSlash(std::string input)
{
  if(input.size())
    {
      char c = input[input.size()-1];
      if(c != '\\' && c != '/')
        input += "/";
    }
  return input;
}

void Dir::fromFS(Hash::DirMap &dir, const std::string &where,
                 Cache::ICacheIndex &cache, bool recurse,
                 bool includeDirs, const std::string &prefix)
{
  assert(where != "");

  /* Find how much we need to strip away from file paths to make them
     relative to 'where'. For example, if 'where' is:

     "/some/dir/"  (10 chars)

     Then filenames will be on the form: "/some/dir/myfile"

     And we need to strip away 10 leading characters to get just
     "myfile".
   */
  int pathlen = addSlash(where).size();

  bf::recursive_directory_iterator iter(where), end;
  for(; iter != end; ++iter)
    {
      std::string file = iter->path().string();

      // Skip deeper levels if the user requested non-recursion
      if(!recurse && iter.level() != 0)
        continue;

      // Find local filename relative to 'where', and include the
      // prefix.
      std::string local = prefix + file.substr(pathlen);

      // Include directory names (with empty hashes) if the user
      // requested it
      if(includeDirs && bf::is_directory(file))
        {
          dir[addSlash(local)] = Hash();
          continue;
        }

      // Otherwise only process normal files
      if(!bf::is_regular_file(file)) continue;

      // Get the file hash
      Hash hash = cache.addFile(file);
      assert(hash.isSet());

      // We're done, add it!
      dir[local] = hash;
    }
}
