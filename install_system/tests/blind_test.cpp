#include "common.cpp"

int main()
{
  SetupTest ss("_blind");

  Hash arc1, dir1, arc2, dir2;

  {
    Setup s("Unpacking arc1 normally");
    // This run is mostly to fetch hashes, so we don't have to
    // hard-code them
    arc1 = s.index("data/arc1.zip");
    dir1 = s.index("data/arc1.dir");
    arc2 = s.index("data/arc2.zip");
    dir2 = s.index("data/arc2.dir");
    s.rules.addArchive(arc1, dir1);
    s.addDir(arc1, "arc1");
  }
  {
    Setup s("Unpacking arc2 BLIND");
    s.index("data/arc2.zip");
    s.rules.addArchive(arc2, dir2);
    s.addDir(arc2, "arc2/");
  }
  ss.print();
  {
    Setup s("Upgrading arc1->arc2 BLIND");
    s.index("data/arc1.zip");
    s.index("data/arc2.zip");
    s.rules.addArchive(arc1, dir1);
    s.rules.addArchive(arc2, dir2);
    s.remDir(arc1, "arc1");
    s.addDir(arc2, "arc1");
  }
  ss.print();
  {
    Setup s("Downgrading arc2->arc1 BLIND");
    s.index("data/arc1.zip");
    s.index("data/arc2.zip");
    s.rules.addArchive(arc1, dir1);
    s.rules.addArchive(arc2, dir2);
    s.remDir(arc2, "arc2");
    s.addDir(arc1, "arc2");
  }
  
  return 0;
}
