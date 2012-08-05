#include "sr0.hpp"

#include <boost/filesystem.hpp>
#include <mangle/stream/servers/outfile_stream.hpp>

#include <iostream>
using namespace std;

using namespace Mangle::Stream;
using namespace Spread;

// This is used to generate the test set. It is NOT part of the test
// itself.
void genData1()
{
  Hash robo("N0VT8AYfLEu2hFufocgj9ykAQoNEgcQwzLW7m1Tfc-cj");
  Hash out1("psjM8uDq9pmNUZQHCFw8zK4-ayXT5-gpFuksShg-GPUo");
  Hash out2("g2lF9D0PACSgcFZUtHpxhrd6jbdnJmfUyXQ0uwO1VZo-");

  Directory dir;
  dir.dir["out_robot.txt"] = robo;
  dir.dir["out_file1.txt"] = out1;
  dir.dir["out_file2.txt"] = out2;
  dir.dir["out_file3.txt"] = out1;

  Hash dirHash = dir.write("_outdir1.dat");
  OutFileStream out("_outdir1.hash");
  out.write(dirHash.getData(), 40);
}

void genData2()
{
  Hash out1("psjM8uDq9pmNUZQHCFw8zK4-ayXT5-gpFuksShg-GPUo");
  Hash out2("2-ZutRu-hTIPSAkddcskIPrYFVYK-J9oFSujhDD7bjEn");
  Hash out3("sJjzhl8Vh7vq-hJYYbvYjPKlvPAWc6olCd3Mk_MMv_Qk");
  Hash arcfile("-M2zyJL1Xt9rJYII9fzgh0gS9tHhklFbOTt1wY__Qy9RAg");

  Directory adir;
  adir.dir["outfile1.txt"] = out1;
  adir.dir["outfile2.txt"] = out2;
  adir.dir["outfile3.txt"] = out3;

  Hash arcdir = adir.write("_arcdir2.dat");

  Directory dir;
  dir.dir["out_file1.txt"] = out1;
  dir.dir["out_file2.txt"] = out2;
  dir.dir["out_file4.txt"] = out3;

  Hash dirHash = dir.write("_outdir2.dat");
  OutFileStream out("_outdir2.hash");
  out.write(dirHash.getData(), 40);
}

void check(JobInfoPtr info)
{
  if(info->isSuccess())
    cout << "SUCCESS: " << info->getMessage() << " prog=" << info->getCurrent()
         << "/" << info->getTotal() << endl;
  else
    cout << "FAILURE: " << info->getMessage() << endl;
}

void test(const std::string &src, const std::string &dest, bool kill=true)
{
  cout << "Installing " << src << " into " << dest << ":\n";
  Cache::Cache cache;
  cache.tmpDir = "_tmpdir/";
  if(kill) boost::filesystem::remove_all(dest);
  check(SR0::fetchFile(src, dest, cache, false));
  cout << endl;
}

int main()
{
  //genData1();
  //genData2();
  //return 0;

  boost::filesystem::remove_all("_tmpdir/");

  test("test1", "_outdir1/");
  test("test1", "_outdir2/");
  test("test1", "_outdir2/", false);
  test("test2", "_outdir3");
  test("test1", "_outdir4_1to2");
  test("test2", "_outdir4_1to2", false);
  test("test2", "_outdir5_2to1");
  test("test1", "_outdir5_2to1", false);

  /* TODO: if/when we implement dir diffing later, we should recheck
     this example to make sure outdated files are actually deleted
     from the outdir4/5 dirs.
   */

  return 0;
}
