#include "common.cpp"
#include <mangle/stream/servers/outfile_stream.hpp>

/* Spread3 test:

   Fully test installing, reinstalling, overwriting, uninstalling,
   upgrading, non-upgrading and downgrading.
 */

bf::path mydir = "_test3/";

boost::shared_ptr<SpreadLib> spr;

void inst(const std::string &where, bool doUpgrade)
{
  cout << "\nINSTALLING to " << where << endl;
  //shortStatus(*spr);
  JobInfoPtr inf = spr->installPack("test", "test", where, NULL, false, doUpgrade);
  if(inf)
    if(inf->isSuccess()) cout << "SUCCESS!\n";
    else inf->failError();
  else cout << "NO JOB EXECUTED\n";
  shortStatus(*spr);
  printDir(where);
}

void uninst(const std::string &where)
{
  cout << "\nUNINSTALLING " << where << endl;
  JobInfoPtr inf = spr->uninstallPack("test", "test", where, false);
  if(inf)
    if(inf->isSuccess()) cout << "SUCCESS!\n";
    else inf->failError();
  else cout << "NO JOB EXECUTED\n";
  shortStatus(*spr);
  if(bf::exists(where)) throw runtime_error("ERROR: dir still exists!\n");
}

void load1()
{
  spr->updateFromFS("test", "input_data/data3/sr0_1", false)->failError();
}

void load2()
{
  spr->updateFromFS("test", "input_data/data3/sr0_2", false)->failError();
}

#define INST inst((where/"test").string(),doUpgrade)
#define UNINST uninst((where/"test").string())
#define WRITE(name,what) Mangle::Stream::OutFileStream::Write((where/"test"/name).string(),what)

void testAll(const bf::path &where, bool doUpgrade)
{
  spr.reset(new SpreadLib((where/"spread").string(), (mydir/"tmp").string()));

  load1();
  INST;

  cout << "\nINSTALLING AGAIN:\n";
  INST;

  cout << "\nMODIFYING FILES:\n";
  WRITE("file4","FILE9");
  WRITE("file1","SOMETHING_LONGER");
  INST;

  cout << "\nREMOVING:\n";
  UNINST;
}

int main()
{
  bf::remove_all(mydir);

  testAll(mydir/"no-upgrade", false);
  testAll(mydir/"upgrade", true);

  return 0;
}
