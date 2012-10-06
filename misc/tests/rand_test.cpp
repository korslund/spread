#include <iostream>
#include "random.hpp"

using namespace std;
using namespace Misc;

void test(Random &r)
{
  cout << "Ints: ";
  for(int i=0; i<10; i++)
    cout << r.gen() << "  ";

  cout << "\nLow ints: ";
  for(int i=0; i<10; i++)
    cout << r.genBelow(20) << "  ";

  cout << "\nDoubles 0-1: ";
  for(int i=0; i<10; i++)
    cout << r.dgen1() << "  ";

  cout << "\n\n";
}

int main(int argc, char **argv)
{
  Random r1(1);
  test(r1);

  Random r2(239874);
  test(r2);

  if(argc >= 2)
    {
      Random r;
      test(r);

      cout << r.genBelow(3) << endl;
    }
  else cout << "Pass any parameter to generate real random numbers\n";

  return 0;
}
