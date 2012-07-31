#include "build_actions.hpp"
#include <iostream>

using namespace std;
using namespace Spread;

Hash hello("hello", 5);
Hash world("world", 5);
Hash testsh("ctEjJBRstghw4_UpmjBdhwJZFl8faISyIeEk2sOH5LLfAQ");
Hash zipfile("bJUN2VkwS-cRpIlPRil-yWIxuoOfiWUG4li9IxH5MGWkAQ");
Hash zipzip("RKEFhXsBxTYXmQt4oMxU_RQk_t7F0Oj8RMGALSJKh6UfAg");
Hash testzip("UXWlJa1SLdf0UcJTqiM8dKWxKOORKX_GlpRhnnT2TOlMAQ");

DirectoryPtr testDir, zipDir, zipzipDir;

Hash testDirH, zipDirH, zipzipDirH;

void setupDirs()
{
  testDir.reset(new Directory);
  zipDir.reset(new Directory);
  zipzipDir.reset(new Directory);

  testDir->dir["hello.out"] = hello;
  testDir->dir["dir/world.out"] = world;
  zipDir->dir["test.sh"] = testsh;
  zipzipDir->dir["testsh.zip"] = zipfile;

  using namespace Mangle::Stream;

  testDirH = testDir->write("_test.dir");
  zipDirH = zipDir->write("_zip.dir");
  zipzipDirH = zipzipDir->write("_zipzip.dir");
}

void print(ActionMap &output)
{
  cout << "Got " << output.size() << " actions:\n";
  ActionMap::iterator it;
  for(it = output.begin(); it != output.end(); it++)
    {
      Action &a = it->second;

      cout << it->first << " : ";

      if(a.isNone())
        {
          cout << "UNHANDLED\n";
        }
      else if(a.isRule())
        {
          const Rule &r = *a.rule;

          cout << r.ruleString << endl;

          // Print rule dependencies
          for(int i=0; i<r.deps.size(); i++)
            cout << "  <= " << r.deps[i] << endl;
        }
      else if(a.isCopy())
        {
          cout << "COPY from " << a.source << endl;
        }
      else assert(0);

      // Print output files
      std::set<std::string>::iterator it2;
      for(it2 = a.destlist.begin(); it2 != a.destlist.end(); it2++)
        cout << "  => " << *it2 << endl;
    }
}

void test(ActionBuilder &build, const std::string &msg)
{
  cout << "\n" << msg << ":\n";
  ActionMap acts;
  try { build.build(acts); }
  catch( std::exception &e )
    {
      cout << "CAUGHT: " << e.what() << endl;
      return;
    }
  print(acts);
}

#define ADD(f,h) assert(cache.index.addFile(f)==(h))

int main()
{
  setupDirs();
  {
    Cache::Cache cache;
    RuleSet rules;
    ActionBuilder build(cache, rules);
    test(build, "Empty builder");
  }

  {
    Cache::Cache cache;
    RuleSet rules;
    ActionBuilder build(cache, rules);
    build.addDep("output.dat", hello);
    test(build, "Request non-existing file");
  }

  {
    Cache::Cache cache;
    RuleSet rules;
    ActionBuilder build(cache, rules, "prefix");
    build.addDep("output.dat", hello);
    ADD("hello.txt",hello);
    test(build, "Request existing file");
  }

  {
    Cache::Cache cache;
    RuleSet rules;
    ActionBuilder build(cache, rules, "prefix");
    cout << "\nAdding non-existing dir:\n";
    try { build.addDir(testDirH); }
    catch( std::exception &e )
      {
        cout << "CAUGHT: " << e.what() << endl;
      }
  }

  {
    Cache::Cache cache;
    RuleSet rules;
    ActionBuilder build(cache, rules, "some/dir/");
    build.addDep("test.sh", testsh);
    build.addHint(zipfile);
    //rules.addArchive(zipfile, zipDirH, "ZIP1");
    test(build, "Add hint but not archive");
  }

  {
    Cache::Cache cache;
    RuleSet rules;
    ActionBuilder build(cache, rules, "some/dir/");
    build.addDep("test.sh", testsh);
    //build.addHint(zipfile);
    rules.addArchive(zipfile, zipDirH, "ZIP1");
    test(build, "Add archive but not hint");
  }

  {
    Cache::Cache cache;
    RuleSet rules;
    ActionBuilder build(cache, rules, "some/dir/");
    build.addDep("test.sh", testsh);
    rules.addArchive(zipfile, zipDirH, "ZIP1");
    build.addHint(zipfile);
    test(build, "Both archive and hint, but no loadable dir");
  }

  {
    Cache::Cache cache;
    RuleSet rules;
    ActionBuilder build(cache, rules, "some/dir/");
    build.addDep("test.sh", testsh);
    ADD("_zip.dir",zipDirH);
    rules.addArchive(zipfile, zipDirH, "ZIP1");
    build.addHint(zipfile);
    test(build, "WITH loadable dir");
  }

  {
    Cache::Cache cache;
    RuleSet rules;
    ActionBuilder build(cache, rules, "some/dir/");
    build.addDep("test.sh", testsh);
    rules.addArchive(zipfile, zipDirH, "ZIP1");
    ADD("_zip.dir",zipDirH);
    ADD("testsh.zip",zipfile);
    build.addHint(zipfile);
    test(build, "Full working case");
  }

  {
    Cache::Cache cache;
    RuleSet rules;
    ActionBuilder build(cache, rules, "out");
    rules.addArchive(zipfile, zipDirH, "ZIP1");
    ADD("_zip.dir",zipDirH);
    ADD("testsh.zip",zipfile);
    build.addDir(zipDirH);
    test(build, "Implicit hinting through addDir");
  }

  {
    Cache::Cache cache;
    RuleSet rules;
    ActionBuilder build(cache, rules, "out");
    rules.addArchive(zipfile, zipDirH, "ZIP1");
    ADD("_zip.dir",zipDirH);
    ADD("testsh.zip",zipfile);
    ADD("hello.txt",hello);
    build.addDir(testDir);
    build.addDir(zipDirH);
    test(build, "Three files from different sources, one missing");
  }

  {
    Cache::Cache cache;
    RuleSet rules;
    ActionBuilder build(cache, rules, "out/");
    rules.addArchive(zipfile, zipDirH, "ZIP1");
    ADD("_zip.dir",zipDirH);
    ADD("testsh.zip",zipfile);
    ADD("hello.txt",hello);
    build.addDir(testDir);
    build.addDir(zipDirH);
    rules.addURL(world, "http://world-url.com/");
    test(build, "Three files from different sources, all present");
  }

  {
    Cache::Cache cache;
    RuleSet rules;
    ActionBuilder build(cache, rules);
    build.addDep("test.sh", hello);
    test(build, "Overwriting existing file with wrong data");
  }

  {
    Cache::Cache cache;
    RuleSet rules;
    ActionBuilder build(cache, rules);
    build.addDep("test.sh", testsh);
    test(build, "Overwriting existing file with right data");
  }

  {
    Cache::Cache cache;
    RuleSet rules;
    ActionBuilder build(cache, rules, "out");
    rules.addArchive(zipfile, zipDirH, "TESTSH ZIP");
    rules.addArchive(testzip, testDirH, "TESTDIR ZIP");
    ADD("_zip.dir",zipDirH);
    ADD("_test.dir",testDirH);
    //ADD("testsh.zip",zipfile);
    rules.addURL(zipfile, "zip1URL");
    ADD("testdir.zip",testzip);
    build.addDir(testDirH);
    build.addDir(zipDirH);
    test(build, "Unzipping two zips into one dir");
  }

  {
    Cache::Cache cache;
    RuleSet rules;
    ActionBuilder build(cache, rules);
    rules.addArchive(zipfile, zipDirH, "TESTSH ZIP");
    rules.addArchive(testzip, testDirH, "TESTDIR ZIP");
    ADD("_zip.dir",zipDirH);
    ADD("_test.dir",testDirH);
    //ADD("testsh.zip",zipfile);
    rules.addURL(zipfile, "zip1URL");
    ADD("testdir.zip",testzip);
    build.addHint(testDirH);
    build.addHint(zipDirH);
    test(build, "Hinting to two zips (no actual deps)");
  }

  {
    Cache::Cache cache;
    RuleSet rules;
    ActionBuilder build(cache, rules);
    rules.addArchive(zipfile, zipDirH, "TESTSH ZIP");
    rules.addArchive(testzip, testDirH, "TESTDIR ZIP");
    ADD("_zip.dir",zipDirH);
    ADD("_test.dir",testDirH);
    //ADD("testsh.zip",zipfile);
    rules.addURL(zipfile, "zip1URL");
    ADD("testdir.zip",testzip);
    build.addHint(testDirH);
    build.addHint(zipDirH);
    build.addDep("file_hello", hello);
    build.addDep("file_world", world);
    build.addDep("file_testsh", testsh);
    build.addDep("file_hello_again", hello);
    test(build, "Adding stand-alone files from two zips");
  }

  {
    Cache::Cache cache;
    RuleSet rules;
    ActionBuilder build(cache, rules);
    rules.addArchive(zipfile, zipDirH, "ZIP");
    rules.addArchive(zipzip, zipzipDirH, "ZIPZIP");
    ADD("_zip.dir",zipDirH);
    ADD("_zipzip.dir",zipzipDirH);
    ADD("zipzip.zip",zipzip);
    build.addHint(zipDirH);
    //build.addHint(zipzipDirH);
    build.addDep("testsh_output", testsh);
    test(build, "Zip in zip, hinting at inner zip");
  }

  {
    Cache::Cache cache;
    RuleSet rules;
    ActionBuilder build(cache, rules);
    rules.addArchive(zipfile, zipDirH, "ZIP");
    rules.addArchive(zipzip, zipzipDirH, "ZIPZIP");
    ADD("_zip.dir",zipDirH);
    ADD("_zipzip.dir",zipzipDirH);
    ADD("zipzip.zip",zipzip);
    //build.addHint(zipDirH);
    build.addHint(zipzipDirH);
    build.addDep("testsh_output", testsh);
    test(build, "Zip in zip, hinting at outer zip");
  }

  {
    Cache::Cache cache;
    RuleSet rules;
    ActionBuilder build(cache, rules);
    rules.addArchive(zipfile, zipDirH, "ZIP");
    rules.addArchive(zipzip, zipzipDirH, "ZIPZIP");
    ADD("_zip.dir",zipDirH);
    ADD("_zipzip.dir",zipzipDirH);
    ADD("zipzip.zip",zipzip);
    build.addHint(zipDirH);
    build.addHint(zipzipDirH);
    build.addDep("testsh_output", testsh);
    test(build, "Zip in zip, hinting at both zips");
  }

  {
    Cache::Cache cache;
    RuleSet rules;
    ActionBuilder build(cache, rules);

    rules.addURL(hello, "url 3 10", 3, 10);
    rules.addURL(hello, "url 4", 4);
    rules.addURL(hello, "url default");
    build.addDep("output.hello", hello);
    test(build, "All URLs present");
    rules.reportBrokenURL(hello, "url 4");
    test(build, "Disabled url 4");
    rules.reportBrokenURL(hello, "url 3 10");
    test(build, "Disabled the other url also");
    rules.reportBrokenURL(hello, "url default");
    test(build, "Disabled everything");
  }

  /* Unhandled case: 'weird' unpacking data. We don't know how, or
     even if, this will be implemented yet.

     This will possibly just be an extra hash parameter to
     addArchive(), passed to ArcRuleSet::addArchive() and stored in
     ArcRule. If it ends up being a Directory object, then we pass it
     as a DirectoryCPtr. Otherwise we can keep it as a hash and load
     it from cache when setting up UnpackHash.
   */

  cout << endl;
  return 0;
}
