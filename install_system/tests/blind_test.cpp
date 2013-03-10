#include "common.cpp"

int main()
{
  SetupTest ss("_blind");

  Hash arc1, dir1, arc2, dir2;

  {
    Setup s("");
    arc1 = s.index("data/arc1.zip");
    dir1 = s.index("data/arc1.dir");
    arc2 = s.index("data/arc2.zip");
    dir2 = s.index("data/arc2.dir");
    s.rules.addArchive(arc1, dir1);
    s.rules.addArchive(arc2, dir2);
  }
  return 0;
}
