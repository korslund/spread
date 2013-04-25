#include "common.cpp"
#include <mangle/stream/servers/outfile_stream.hpp>

/* Spread3 test:

   Fully test installing, reinstalling, overwriting, uninstalling,
   upgrading, non-upgrading and downgrading.

   Note that the second series of tests (data4) uses packages with
   several dirs, in different order. This test produces the SAME
   result in ALL install cases, because the order of dirs is
   irrelevant. Whenever two dirs in one package overwrite the same
   file location, the result is UNDEFINED, so this works exactly as
   expected.

   We may fix this later so that the last dir always takes precedence,
   which will change the output of this test (and actually make it a
   more useful test.)
 */

bf::path mydir = "_test3/";

boost::shared_ptr<SpreadLib> spr;

void inst(const std::string &where, bool doUpgrade)
{
  cout << "\nINSTALLING to " << where << endl;
  JobInfoPtr inf = spr->installPack("test", "test", where, NULL, false, doUpgrade);
  if(inf)
    if(inf->isSuccess()) cout << "SUCCESS!\n";
    else inf->failError();
  else cout << "NO JOB EXECUTED\n";
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

void load1(const std::string &base)
{
  spr->updateFromFS("test", "input_data/" + base + "/sr0_1", false)->failError();
  shortStatus(*spr);
}

void load2(const std::string &base)
{
  spr->updateFromFS("test", "input_data/" + base + "/sr0_2", false)->failError();
  shortStatus(*spr);
}

#define INST inst((where/"test").string(),doUpgrade)
#define UNINST uninst((where/"test").string())
#define WRITE(name,what) Mangle::Stream::OutFileStream::Write((where/"test"/name).string(),what)
#define LOAD1 load1(sr0src)
#define LOAD2 load2(sr0src)

void testAll(const std::string &subdir, bool doUpgrade, const std::string &sr0src)
{
  bf::path where = mydir/sr0src/subdir;

  spr.reset(new SpreadLib((where/"spread").string(), (mydir/"tmp").string()));

  LOAD1;
  INST;

  cout << "\nINSTALLING AGAIN:\n";
  INST;

  cout << "\nMODIFYING FILES:\n";
  if(sr0src == "data3")
    {
      WRITE("file4","FILE9");
      WRITE("file1","SOMETHING_LONGER");
    }
  INST;

  cout << "\nMODIFYING IGNORED FILE:\n";
  if(sr0src == "data3")
    WRITE("file3","hello");
  INST;

  cout << "\nREMOVING:\n";
  UNINST;

  cout << "\nSWITCHING TO CHANNEL 2\n";
  LOAD2;
  INST;

  cout << "\nUPGRADING TO 1\n";
  LOAD1;
  INST;

  cout << "\nBACK to 2\n";
  LOAD2;
  INST;

  cout << "\nAND AGAIN\n";
  INST;
  UNINST;

  cout << "\nFRESH INSTALL:\n";
  INST;
  cout << "\nMODIFY AND UPDATE:\n";
  LOAD1;
  if(sr0src == "data3")
    {
      WRITE("file4","FILE9");
      WRITE("file1","SOMETHING_LONGER");
      WRITE("file3","hello");
    }
  INST;
}

void testBoth(const std::string &sr0)
{
  testAll("no-upgrade", false, sr0);
  testAll("upgrade", true, sr0);
}

int main()
{
  bf::remove_all(mydir);

  testBoth("data3");
  testBoth("data4");

  return 0;
}
