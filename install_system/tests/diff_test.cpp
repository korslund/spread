#include "common.cpp"

int main()
{
  SetupTest ss("_diff");

  {
    Setup s("Remove non-existing file");
    s.remFile("hello", hello);
  }
  {
    Setup s("Moving non-existing file");
    s.index("data/hello.dat");
    s.remFile("hello", hello);
    s.addFile("hello2", hello);
  }
  ss.print();
  {
    Setup s("Moving existing file");
    s.remFile("hello2", hello);
    s.addFile("hello", hello);
  }
  ss.print();
  {
    Setup s("Ignoring file (with different hash)");
    s.remFile("hello", world);
    s.addFile("hello", world);
  }
  ss.print();
  {
    Setup s("Replacing file");
    s.index("data/world.dat");
    s.remFile("hello", hello);
    s.addFile("hello", world);
  }
  Hash robot;
  ss.print();
  {
    Setup s("Deleting file");
    s.index("data/world.dat");
    robot = s.index("data/robot.dat");
    s.remFile("hello", world);
  }
  ss.print();

  Hash::DirMap dir2 = dir1;
  dir2["abc"] = world;
  dir2.erase("jkl/mno");
  dir2["jkl/mno_NEW"] = robot;
  {
    Setup s("Installing dir");
    s.index("data/hello.dat");
    s.index("data/world.dat");
    s.addDir(dir1);
  }
  ss.print();
  {
    Setup s("Installing dir again");
    s.addDir(dir1);
  }
  ss.print();
  {
    Setup s("Installing dir again, expecting no changes");
    s.addDir(dir1);
    s.remDir(dir1);
  }
  ss.print();
  {
    Setup s("Moving to subdir");
    s.addDir(dir1, "subdir");
    s.remDir(dir1);
  }
  ss.print();
  {
    Setup s("Moving back");
    s.addDir(dir1);
    s.remDir(dir1, "subdir");
  }
  ss.print();
  {
    Setup s("User-modding a single file");
    s.index("data/world.dat");
    s.addFile("def/ghi", world);
    s.remFile("def/ghi", hello);
  }
  ss.print();
  {
    Setup s("Installing dir again, expecting no changes");
    s.addDir(dir1);
    s.remDir(dir1);
  }
  ss.print();
  {
    Setup s("Upgrading dir");
    s.index("data/robot.dat");
    s.addDir(dir2);
    s.remDir(dir1);
  }
  ss.print();
  {
    Setup s("Downgrading dir");
    s.index("data/hello.dat");
    s.addDir(dir1);
    s.remDir(dir2);
  }
  ss.print();
  {
    Setup s("Swap two files");
    s.addFile("abc", world);
    s.remFile("abc", hello);
    s.addFile("def/ghi", hello);
    s.remFile("def/ghi", world);
  }

  return 0;
}
