#include "math/random.h"

#include <random>

using namespace indigo;

Random::Random(unsigned int seed) : _rng(seed)
{
}

unsigned int Random::next(const unsigned int min, const unsigned int max)
{
    std::uniform_int_distribution<unsigned int> uniform_dist(min, max);
    return uniform_dist(_rng);
}

unsigned int Random::nextMod(const int mod)
{
    if (mod > 0)
    {
        return next() % mod;
    }
    if (mod < 0)
    {
        return -(next() % -mod);
    }
    return 0;
}

double Random::nextDouble()
{
    std::uniform_real_distribution<> uniform_dist;
    return uniform_dist(_rng);
}
