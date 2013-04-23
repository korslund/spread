#include "common.cpp"

/* Spread2 test:

   Test channel loading from SR0 (Spread-REST v0) repositories. Loads
   channels and tests the effect on installed packages.
 */

bf::path mydir = "_test2/";

boost::shared_ptr<SpreadLib> s;

void setup()
{
  bf::remove_all(mydir/"channels");
  s.reset(new SpreadLib(mydir.string(), (mydir/"tmp").string()));
  s->cacheFile("input_data/data2/dir1.zip");
  s->cacheFile("input_data/data2/dir2.zip");
}

void p()
{
  printDir(mydir.string(), true);
  if(s) shortStatus(*s);
}

void load(const std::string &chan, const std::string &which)
{
  cout << "Loading chan: " << chan << endl;
  cout << "  pre-wasUpdated: " << s->wasUpdated(chan) << endl;
  s->updateFromFS(chan, "input_data/data2/sr" + which, false)->failError();
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

  /*
  {
    S("LOADING 1 twice");
    load1();
    load1();
  }
  */

  return 0;
}
