#include "common.cpp"

/* Spread2 test:

   Test channel loading from SR0 (Spread-REST v0) repositories. Loads
   channels and tests the effect on installed packages.
 */

bf::path mydir = "_test2/";
bf::path chandir = mydir/"channels";

boost::shared_ptr<SpreadLib> s;

void setup()
{
  bf::remove_all(chandir);
  s.reset(new SpreadLib(mydir.string(), (mydir/"tmp").string()));
  s->cacheFile("input_data/data2/dir1.zip");
  s->cacheFile("input_data/data2/dir2.zip");
  //s->getJobManager()->setPrintLogger();
}

void p()
{
  if(bf::exists(chandir))
    printDir(chandir.string());
  if(s) shortStatus(*s);
}

void load(const std::string &chan, const std::string &which)
{
  cout << "Loading chan: " << chan << endl;
  cout << "  pre-wasUpdated: " << s->wasUpdated(chan) << endl;
  JobInfoPtr info = s->updateFromFS(chan, "input_data/data2/sr" + which, false);
  info->failError();
  assert(info->isSuccess());
  cout << "  wasUpdated: " << s->wasUpdated(chan) << endl;
}

void load1(const std::string &chan="a") { load(chan, "1"); }
void load2(const std::string &chan="a") { load(chan, "2"); }

struct SS
{
  SS(const std::string &msg) { cout << endl << msg << ":\n"; setup(); }
  ~SS() { p(); }
};

#define S SS ss

int main()
{
  bf::remove_all(mydir);
  bf::create_directories(mydir);
  bf::copy_file("input_data/data2/fakeinst.conf", mydir/"installed.conf");

  { S("EMPTY TEST"); }

  {
    S("LOADING 1");
    load1();
  }

  {
    S("LOADING 2");
    load2();
  }

  {
    S("LOADING B1");
    load1("b");
  }

  {
    S("LOADING B2");
    load2("b");
  }

  {
    S("LOADING 1 twice");
    load1();
    load1();
  }

  {
    S("LOADING 2 twice");
    load2();
    load2();
  }

  {
    S("LOADING A and B");
    load2("b");
    load1("a");
  }

  {
    S("LOADING 1->2");
    load1();
    load2();
  }

  {
    S("LOADING 2->1");
    load2();
    load1();
  }

  {
    S("LOADING 2->1->2");
    load2();
    load1();
    load2();
  }

  {
    S("LOADING 1->2->2->1");
    load1();
    load2();
    load2();
    load1();
  }

  return 0;
}
