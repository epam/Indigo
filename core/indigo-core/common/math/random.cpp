#include "math/random.h"

#include <random>

using namespace indigo;

Random::Random(int seed)
{
    _rng = std::make_unique<pcg32_fast>(seed);
}

int Random::next()
{
    std::uniform_int_distribution<int> uniform_dist(0);
    return uniform_dist(*_rng);
}

int Random::next(int mod)
{
    std::uniform_int_distribution<int> uniform_dist(-mod, mod);
    return uniform_dist(*_rng);
}

double Random::nextDouble()
{
    std::uniform_real_distribution<double> uniform_dist;
    return uniform_dist(*_rng);
}
