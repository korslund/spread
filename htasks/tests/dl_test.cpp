#include "downloadhash.hpp"

#include <iostream>
using namespace std;
using namespace Spread;

int main()
{
  string url = "http://tiggit.net/robots.txt";
  cout << "Downloading " << url << " ...\n";

  Hash hash("=kK;wq4^4@_~O9[XkYLynWo6R7U>c6W/O+ZdU)ItK");

  DownloadHash dl;
  dl.url = url;
  dl.addOutput(hash, "_robots.txt");
  dl.addOutput(hash, "_robots2.txt");
  dl.run(false);

  return 0;
}
