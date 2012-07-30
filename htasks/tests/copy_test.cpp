#include "copyhash.hpp"

#include <iostream>
using namespace std;
using namespace Spread;

int main()
{
  string source = "test1.zip";
  cout << "Copying " << source << " ...\n";

  Hash hash("OEa8aUtr8n77FuIrXefgKVpVRY66g2CULBYP4N9EuQIEBw");

  CopyHash dl;
  dl.source = source;
  dl.addOutput(hash, "_copy1.zip");
  dl.addOutput(hash, "_copy2.dat");
  dl.run(false);

  return 0;
}
