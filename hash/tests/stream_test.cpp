#include <iostream>
using namespace std;

#include "hash_stream.hpp"

using namespace Spread;

void doSum(const std::string &file)
{
  // First, create an MD5Stream to sum the file while reading.
  HashStreamPtr str(new HashStream(file));

  // Then sum the entire file using sum()
  Hash sum = str->sum();

  // Check that the two sums agree
  assert(sum == str->finish());

  cout << sum << " " << file << endl;
  cout << sum.toHex() << " " << file << endl;
}

int main(int argc, char **argv)
{
  if(argc > 1)
    {
      for(int i=1; i<argc; i++)
        doSum(argv[i]);
    }
  else
    doSum("test.sh");

  return 0;
}
