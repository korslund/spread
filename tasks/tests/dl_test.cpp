#include "../download.hpp"

#include <iostream>
using namespace std;

int main()
{
  string url = "http://tiggit.net/client/latest.tig";
  cout << "Downloading " << url << "...\n";

  Tasks::DownloadTask dlj(url, "_output.txt");
  Jobs::JobInfoPtr info = dlj.getInfo();

  dlj.run();

  if(info->isSuccess())
    cout << "Success!\n";
  else
    cout << "Failure: " << info->getMessage() << endl;

  cout << "Progress: " << info->getCurrent() << "/" << info->getTotal() << endl;

  return 0;
}
