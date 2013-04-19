#include <iostream>
#include "../../spread.hpp"
#include <boost/filesystem.hpp>
#include "print_dir.hpp"

using namespace std;
using namespace Spread;

int main()
{
  boost::filesystem::remove_all("_out");

  SpreadLib spread("_out", "_tmp");

  cout << "Copying files:\n";

  JobInfoPtr info(new JobInfo);
  vector<string> src, dst;
  src.push_back("test.sh");
  src.push_back("bug1_test.cpp");
  dst.push_back("_copy_many/file1");
  dst.push_back("_copy_many/file2");

  spread.cacheCopy(src, dst, info);
  cout << "Progress: " << info->getCurrent() << "/" << info->getTotal() << endl;

  printDir("_copy_many");
  //printDir("_out");

  return 0;
}
