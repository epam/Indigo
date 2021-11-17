#pragma once

#include <memory>

#include <pcg_random.hpp>

namespace indigo
{
    class Random
    {
    private:
        pcg32_fast _rng;

    public:
        explicit Random(unsigned int seed = 0);

        unsigned int next(unsigned int min = 0, unsigned int max = std::numeric_limits<unsigned int>::max());
        unsigned int nextMod(int mod);

        double nextDouble();
    };
}
