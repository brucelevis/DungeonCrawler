#include <random>
#include <GameLib/Random.h>

namespace gamelib
{
  int random_range(int low, int high)
  {
    static std::random_device rd;

    std::mt19937 rng { rd() };
    std::uniform_int_distribution<int> gen { low, high };

    return gen(rng);
  }
}
