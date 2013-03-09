#include <iostream>

#include "index.hpp"
#include <map>

using namespace Spread;
using namespace std;

Hash hello("hello", 5);
Hash world("HelloWorld", 10);
Hash wrong("WRONG");

struct MyFS : Cache::FSystem
{
  std::string abs(const std::string &file) { return file; }
  bool exists(const std::string &file)
  { return file == "file1" || file == "file2"; }

  uint64_t file_size(const std::string &file)
  { if(file == "file1") return world.size(); return hello.size(); }

  bool equivalent(const std::string &file1, const std::string &file2)
  { return file1 == file2; }

  uint64_t last_write_time(const std::string &file) { return 1; }
  Spread::Hash hashSum(const std::string &file)
  {
    cout << "SUMMING " << file << endl;
    if(file == "file1") return world;
    if(file == "file2") return hello;
    assert(0);
  }
};

MyFS fs;

Cache::CacheIndex index("", &fs);

void add(const std::string &file, const Hash &h = Hash(), bool allowMissing=false)
{
  cout << "Adding " << file;
  if(!h.isNull()) cout << " (expecting " << h << ")";
  cout << endl;
  try
    {
      Hash out =index.addFile(file,h, allowMissing);
      cout << "  Got: " << out << endl;
    }
  catch(exception &e)
    {
      cout << "  Error: " << e.what() << endl;
    }
}

void find(const Hash &h)
{
  cout << "Searching for " << h << endl;
  cout << "  Found: " << index.findHash(h) << endl;
}

void status(const std::string &file, const Hash &h)
{
  using namespace Cache;

  cout << "Checking status for " << file << "   Hash: " << h << "\n  ";
  switch(index.getStatus(file,h))
    {
    case CI_None: cout << "No file found\n"; break;
    case CI_Match: cout << "File already matches\n"; break;
    case CI_Diff: cout << "File exists but is different:\n    "
                       << index.addFile(file) << endl; break;
    case CI_ElseWhere: cout << "Hash exists elsewhere: " << index.findHash(h) << endl;
      break;
    default:
      assert(0);
    }
}

int main()
{
  cout << "ADDING:\n";
  add("missing");
  add("missing", wrong);
  add("missing", Hash(), true);
  add("missing", wrong, true);
  add("file1");
  add("file1", wrong);
  add("file1", world);
  add("file2", hello);

  cout << "\nFINDING:\n";
  find(wrong);
  find(hello);
  index.removeFile("file2");
  find(hello);
  index.removeFile("file2");
  find(hello);

  cout << "\nSTATUS:\n";
  status("file1", hello);
  status("file1", world);
  status("file1", wrong);
  status("file2", hello);
  status("file2", world);
  status("file2", wrong);

  cout << "\nCACHE CONTENTS:\n";
  Cache::CIVector vec;
  index.getEntries(vec);
  for(int i=0; i<vec.size(); i++)
    {
      Cache::CIEntry &e = vec[i];
      cout << e.hash << " " << e.file << " " << e.writeTime << endl;
    }

  return 0;
}
