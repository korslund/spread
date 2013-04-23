#include "common.cpp"

/* Spread1 test: basic function test. Testing various function calls
   and error handling.
 */

bf::path mydir = "_test1/";
bf::path spdir = mydir/"spread";
std::string outdir = (mydir/"out").string();

#define S cout<<endl;bf::remove_all(mydir); try
#define P SpreadLib s(spdir.string(), (mydir/"tmp").string())
#define X catch(exception &e) { cout << "CAUGHT: " << e.what(); }printDir(mydir.string(),true);

#define INST(X) s.installPack("c",X,outdir,&ver,false,false)

int main()
{
  string ver;
  S { P; } X;
  S
    {
      P;
      INST("p");
    } X;
  S
    {
      P;
      s.cacheCopy("input_data/data1/rules.json", (spdir/"channels/c/rules.json").string());
      s.cacheCopy("input_data/data1/packs.json", (spdir/"channels/c/packs.json").string());
      print(s.getInfoList("c"));
      INST("NOPACK");
      cout << "Ver=" << ver << endl;
    } X;
  S
    {
      P;
      s.cacheCopy("input_data/data1/rules.json", (spdir/"channels/c/rules.json").string());
      s.cacheCopy("input_data/data1/packs.json", (spdir/"channels/c/packs.json").string());
      INST("p");
      assert(0); // Should never get here since "p" does not have a
                 // valid dir hash and installPack() should throw.
    } X;
  S
    {
      P;
      s.cacheCopy("input_data/data1/rules.json", (spdir/"channels/c/rules.json").string());
      s.cacheCopy("input_data/data1/packs.json", (spdir/"channels/c/packs.json").string());
      s.cacheFile("input_data/data1/file1");
      s.cacheFile("input_data/data1/file2");
      s.cacheFile("input_data/data1/dir1.dat");
      print(s.getPackStatus("c","p2"));
      INST("p2");
      print(s.getPackStatus("c","p"));
      print(s.getPackStatus("c","p2"));
      printStatus(s);
    } X;
  S
    {
      P;
      s.cacheCopy("input_data/data1/rules.json", (spdir/"channels/c/rules.json").string());
      s.cacheCopy("input_data/data1/packs.json", (spdir/"channels/c/packs.json").string());
      cout << "ARCHIVE: " << s.cacheFile("input_data/data1/arc1.zip") << endl;
      INST("p3");
      printStatus(s);
    } X;
  S
    {
      bf::create_directories(spdir);
      bf::copy_file("input_data/data1/fakeinst.conf", spdir/"installed.conf");
      P;
      printStatus(s);
      cout << "INDIVIDUAL PACKS:\n";
      print(s.getPackStatus("a", "1"));
      print(s.getPackStatus("a", "1", "/dir4"));
      print(s.getPackStatus("b", "1"));
      print(s.getPackStatus("b", "2"));
      cout << "\nLISTS:\n";
      printStatus(s, "a");
      printStatus(s, "b");
      printStatus(s, "", "1");
      printStatus(s, "", "2");
      printStatus(s, "", "3");
      printStatus(s, "", "4");
      printStatus(s, "a", "2");
      printStatus(s, "b", "2");
      printStatus(s, "a", "1");
      cout << "XYZ\n";
      printStatus(s, "a", "1", "/dir1");
      printStatus(s, "a", "1", "/dir2");
      printStatus(s, "a", "1", "/dir4");
      printStatus(s, "", "1", "/dir4");
      printStatus(s, "a", "", "/dir2");
      cout << "ABC\n";
      printStatus(s, "", "", "/dir1");
      printStatus(s, "", "", "/dir2");
      printStatus(s, "", "", "/dir3");
      printStatus(s, "", "", "/dir4");
      printStatus(s, "", "", "/dirX");
      printStatus(s, "a", "", "/dirX");
    } X;

  return 0;
}
