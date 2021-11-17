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

        void setSeed(unsigned int seed);

        unsigned int next();
        unsigned int next(unsigned int max);
        unsigned int next(unsigned int min, unsigned int max);

        unsigned int nextMod(int mod);

        double nextDouble();
    };
}
