#include "downloadhash.hpp"

#include <iostream>
using namespace std;
using namespace Spread;
using namespace Jobs;

int main()
{
  string url = "http://tiggit.net/robots.txt";
  cout << "Downloading " << url << " ...\n";

  Hash hash("N0VT8AYfLEu2hFufocgj9ykAQoNEgcQwzLW7m1Tfc-cj");

  DownloadHash dl;
  dl.url = url;
  dl.addOutput(hash, "_robots.txt");
  dl.addOutput(hash, "_robots2.txt");
  JobInfoPtr info = dl.run(false);
  cout << info->getCurrent() << "/" << info->getTotal() << endl;

  return 0;
}
