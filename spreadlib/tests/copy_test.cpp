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

  cout << "Copying file:\n";
  std::string hsh = spread.cacheCopy("test.sh", "_copy/hello.txt");
  cout << "Result: " << hsh << endl;

  printDir("_copy");
  printDir("_out");

  return 0;
}
