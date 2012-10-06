#include <time.h>

class Timer
{
  clock_t start;

  float toSec(clock_t time)
  {
    return (time-start)*1.0/CLOCKS_PER_SEC;
  }

public:
  Timer() { reset(); }
  void reset() { start = clock(); }
  float total() { return toSec(clock()); }
};
