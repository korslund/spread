#include "common.cpp"

bf::path mydir = "_legacy/";
bf::path chandir = mydir/"channels/test/";
bf::path indir = "input_data/legacy_test/";

int main()
{
  bf::remove_all(mydir);
  bf::create_directories(chandir);
  bf::copy_file(indir/"packs.json", chandir/"packs.json");

  {
    cout << "Setting test2 with no dir file\n";
    SpreadLib lib(mydir.string(), (mydir/"tmp").string());
    lib.setLegacyPack("test", "test2", (mydir/"out").string());
    printDir(mydir.string(), true);
  }
  cout << "Restarting spread\n";
  SpreadLib lib(mydir.string(), (mydir/"tmp").string());
  cout << "Setting test1 WITH dir file\n";
  lib.cacheFile((indir/"test.dir").string());
  lib.setLegacyPack("test", "test1", (mydir/"out").string());
  printDir(mydir.string(), true);
  cout << "Calling again\n";
  lib.setLegacyPack("test", "test1", (mydir/"out").string());
  printDir(mydir.string(), true);
  return 0;
}
