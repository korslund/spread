#include "../unpack.hpp"

#include "print_dir.hpp"

#include <iostream>
using namespace std;

int main()
{
  string zip = "../../unpack/tests/archives/test.zip";
  cout << "Unpacking " << zip << "...\n";

  Spread::UnpackTask unp(zip, "_outdir1");
  Spread::JobInfoPtr info = unp.getInfo();

  unp.run();

  if(info->isSuccess())
    cout << "Success!\n";
  else
    cout << "Failure: " << info->getMessage() << endl;

  cout << "Progress: " << info->getCurrent() << "/" << info->getTotal() << endl;

  printDir("_outdir1");

  return 0;
}
