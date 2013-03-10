#include "common.cpp"

int main()
{
  SetupTest ss("_unpack");

  Hash arcHash;
  Hash dirHash;

  {
    Setup s("Unpack with arcrule but no hint");
    arcHash = s.index("data/arc1.zip");
    dirHash = s.index("data/arc1.dir");
    s.rules.addArchive(arcHash, dirHash);

    s.addFile("test1/hello", hello);
    s.addFile("test1/world", world);
    s.addFile("test1/arc", arcHash);
  }
  {
    Setup s("Unpack with hint but no arcrule");
    s.index("data/arc1.zip");
    s.addHint(arcHash);

    s.addFile("test1/hello", hello);
    s.addFile("test1/world", world);
    s.addFile("test1/arc", arcHash);
  }
  {
    Setup s("Unpack with hint AND arcrule - BLIND");
    s.index("data/arc1.zip");
    s.rules.addArchive(arcHash, dirHash);
    s.addHint(arcHash);

    s.addFile("test1/hello", hello);
    s.addFile("test1/world", world);
    s.addFile("test1/arc", arcHash);
  }
  {
    Setup s("Unpack with hint AND arcrule - NON-BLIND");
    s.index("data/arc1.zip");
    s.index("data/arc1.dir");
    s.rules.addArchive(arcHash, dirHash);
    s.addHint(arcHash);

    s.addFile("test2/hello", hello);
    s.addFile("test2/world", world);
    s.addFile("test2/arc", arcHash);
  }
  {
    Setup s("Unpack zip dir itself - BLIND");
    s.index("data/arc1.zip");
    //s.index("data/arc1.dir");
    s.rules.addArchive(arcHash, dirHash);
    s.addDir(arcHash, "test3");
  }
  {
    Setup s("Unpack zip dir itself - NON-BLIND");
    s.index("data/arc1.zip");
    s.index("data/arc1.dir");
    s.rules.addArchive(arcHash, dirHash);
    s.addDir(arcHash, "test4");
  }
  {
    Setup s("Combo");
    s.index("data/arc1.zip");
    s.index("data/arc1.dir");
    s.index("data/world.dat");
    s.rules.addArchive(arcHash, dirHash);
    s.addDir(arcHash, "test5/sub1");
    s.addDir(dirHash, "test5/sub2");
    s.addFile("test5/sub3/hello", hello);
  }
  return 0;
}
