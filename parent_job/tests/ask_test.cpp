#include <iostream>
#include "askqueue.hpp"
//#include "common.cpp"

using namespace std;
using namespace Spread;

/*
  Things to test:

  - basic case with pushWait() and pop(), check that answers gets
    through

  - test all the UserAsk types, which we might rename to UserMsg types
    now

  - test the reciever end, and the code that's responsible for
    decoding the type of message etc. See if you can simplify it.

  - do a push()-and-ignore test as well. Which basically means you can
    use this for one-way messages. So actually we can make a BASE
    class that JUST has a message. push() uses that, as does pop().

  - test aborting through the response
  - test aboring job while waiting for response

  - test object lifetime in all cases, by using a custom msg type with
    a talkative destructor.
 */

int main()
{
  cout << "Hello\n";

  return 0;
}
