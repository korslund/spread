#include <iostream>
#include "index.hpp"

using namespace Spread;
using namespace std;

Hash hello("hello",5);

struct MyFS : Cache::FSystem
{
  bool found;

  std::string abs(const std::string &file) { return file; }
  bool exists(const std::string &file)
  { return found && file == "file1"; }

  uint64_t file_size(const std::string &file)
  { return 5; }

  bool equivalent(const std::string &file1, const std::string &file2)
  { return file1 == file2; }

  uint64_t last_write_time(const std::string &file) { return 1; }
  Spread::Hash hashSum(const std::string &file)
  {
    if(file == "file1") return hello;
    assert(0);
  }
};

MyFS fs;
Cache::CacheIndex index("", &fs);

void dump()
{
  cout << "INDEX:\n";
  Cache::CIVector vec;
  index.getEntries(vec);
  for(int i=0; i<vec.size(); i++)
    {
      Cache::CIEntry &e = vec[i];
      cout << e.hash << " " << e.file << " " << e.writeTime << endl;
    }
  cout << endl;
}

int main()
{
  fs.found = true;

  Hash::DirMap files;

  cout << "Done nothing yet:\n";
  cout << "File1: " << files["file1"] << endl;
  dump();

  cout << "Checking files:\n";
  index.checkMany(files);
  cout << "File1: " << files["file1"] << endl;
  dump();

  cout << "Removing file and checking again:\n";
  fs.found = false;
  index.checkMany(files);
  cout << "File1: " << files["file1"] << endl;
  dump();

  return 0;
}
