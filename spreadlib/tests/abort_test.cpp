#include "../../spread.hpp"
#include <iostream>
#include <boost/filesystem.hpp>
#include <job/thread.hpp>

/* Regresssion: aborting an install job would set it in an isError()
   state (with an error message caused by failError() being called on
   an aborted JobInfo), instead of isAbort() which is what you expect.
 */

using namespace std;
using namespace Spread;
namespace bf = boost::filesystem;

std::string path = "_abort/";
std::string game = "_abort/game/";

int main()
{
  if(bf::exists(game))
    bf::remove_all(game);

  SpreadLib spread(path, path + "tmp/");
  //spread.getJobManager()->setPrintLogger();

  cout << "Fetching channel\n";
  JobInfoPtr info = spread.updateFromURL("tiggit.net", "http://tiggit.net/client/sr0/", true);
  while(!info->isFinished()) Thread::sleep(0.1);
  info->failError();
  cout << "Installing package\n";
  info = spread.installPack("tiggit.net", "tiggit.net/cave-story", game);
  cout << "Waiting a bit\n";
  Thread::sleep(1.5);
  cout << "Aborting and waiting for finish\n";
  info->abort();
  while(!info->isFinished()) Thread::sleep(0.1);
  cout << "Sleeping a little longer\n";
  Thread::sleep(0.2);
  cout << "Checking for error message\n";
  if(info->isError())
    cout << "Error: " << info->getMessage() << endl;
  else assert(info->isAbort());
}
