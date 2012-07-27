#ifndef __MISC_RANDOM_HPP_
#define __MISC_RANDOM_HPP_

#include <stdint.h>
#include <ctime>

/* Making your own random generator is usually a bad idea with bad
   results, and this is no exception.

   I just needed something stupidly simple that is NOT rand()/srand()
   (for thread safety) and which doesn't depend on Boost or TR1 or
   complicated dependencies. This is mostly just used to pick between
   weighted URL sources, so we REALLY don't need anything good here.
 */
namespace Misc
{
  struct Random
  {
    Random() { seedTime(); }
    Random(uint32_t s) { seed(s); }

    uint32_t gen()
    {
      return state = (69069*state) + 362437;
    }

    void seed(uint32_t i) { state = i; }
    void seedTime()
    {
      uint32_t t = std::time(0);
      seed(t*t);
    }

    uint32_t genBelow(uint32_t max)
    { return gen() % max; }

    // [0..1)
    double dgen()  { return gen() / ((double)0xFFFFFFFF+1); }
    // [0..1]
    double dgen1() { return gen() / ((double)0xFFFFFFFF); }

  private:
    uint32_t state;
  };
}

#endif
