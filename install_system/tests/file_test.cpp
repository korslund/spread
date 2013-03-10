#include "common.cpp"

int main()
{
  SetupTest ss("_file");

  {
    Setup s("Empty run");
  }
  {
    Setup s("Add files, no source");
    s.addFile("test1/hello", hello);
    s.addFile("test1/world", world);
  }
  {
    Setup s("Add files, with source");
    s.addFile("test2/hello", hello);
    s.addFile("test2/world", world);
    s.index("data/hello.dat");
    s.index("data/world.dat");
  }
  {
    Setup s("Same file repeated");
    s.index("data/hello.dat");
    s.addFile("test3/hello", hello);
    s.addFile("test3/hello2", hello);
    s.addFile("test3/dir/hello3", hello);
  }
  {
    Setup s("Overwriting existing file that matches");
    s.index("data/hello.dat");
    s.addFile("test3/hello", hello);
  }
  {
    Setup s("Overwriting ditto, but without having file in cache");
    s.addFile("test3/hello", hello);
  }
  {
    Setup s("Adding a custom dir");
    s.index("data/hello.dat");
    s.index("data/world.dat");
    s.addDir(dir1, "test4");
  }
  {
    Setup s("Overwriting without cache");
    s.addDir(dir1, "test4");
  }
  {
    Setup s("Adding dir twice + subdir");
    s.index("data/hello.dat");
    s.index("data/world.dat");
    s.addDir(dir1, "test5");
    s.addDir(dir1, "test5");
    s.addDir(dir1, "test5/sub");
  }
  {
    Setup s("Adding dir as hash, no source");
    s.addDir(Hash("NOFILE"), "test6");
  }
  {
    Setup s("Adding dir as hash, with source but missing files");
    Hash dirH = s.index("data/dir1.dat");
    s.addDir(dirH, "test6");
  }
  {
    Setup s("Adding dir as hash, should work now");
    Hash dirH = s.index("data/dir1.dat");
    s.index("data/hello.dat");
    s.index("data/world.dat");
    s.addDir(dirH, "test6");
  }

  return 0;
}
