#include <iostream>
#include "spread.hpp"
#include <boost/filesystem.hpp>

using namespace std;
using namespace Spread;
namespace bf = boost::filesystem;

/* Regression test.
 */

int main(int argc, char **argv)
{
  cout << "Supply any parameter to enable debug logging\n\n";

  SpreadLib spread("_bug1", "_tmp");
  if(argc > 1) spread.getJobManager()->setPrintLogger();

  if(bf::exists("_bug1"))
    bf::remove_all("_bug1");

  JobInfoPtr info, info2;

  cout << "Staring update job\n";
  info = spread.updateFromURL("tiggit.net", "http://tiggit.net/client/sr0/", true);

  /* This should fail as there is no loaded data yet.
   */
  bool fail = false;
  try
    {
      cout << "Attempting to install a package\n";
      info2 = spread.installPack("tiggit.net", "tiggit.net/cave-story", "_bug1/cave-story",
                                 NULL, true, false);
      info2->failError();
      cout << "Unexpected success\n";
    }
  catch(exception &e)
    {
      cout << "EXPECTED error: " << e.what() << endl;
      fail = true;
    }
  assert(fail);

  cout << "Waiting for update to finish\n";
  info->wait();

  cout << "Repeating update job\n";
  info = spread.updateFromURL("tiggit.net", "http://tiggit.net/client/sr0/", true);

  /* This should WORK, since there is now available data on disk.
   */
  try
    {
      cout << "Installing package for real this time\n";
      info2 = spread.installPack("tiggit.net", "tiggit.net/cave-story", "_bug1/cave-story",
                                 NULL, true, false);
      cout << "Waiting...\n";
      info2->wait();
      info2->failError();
      cout << "Success!\n";
      if(argc == 1)
        bf::remove_all("_bug1");
    }
  catch(exception &e)
    {
      cout << "ERROR: " << e.what() << endl;
    }

  return 0;
}
