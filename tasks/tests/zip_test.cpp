#include "../unpack.hpp"

#include <iostream>
using namespace std;

int main()
{
  string zip = "../../unpack/tests/archives/test.zip";
  cout << "Unpacking " << zip << "...\n";

  Tasks::UnpackTask unp(zip, "_outdir1");
  Jobs::JobInfoPtr info = unp.getInfo();

  unp.run();

  if(info->isSuccess())
    cout << "Success!\n";
  else
    cout << "Failure: " << info->getMessage() << endl;

  cout << "Progress: " << info->getCurrent() << "/" << info->getTotal() << endl;

  return 0;
}
