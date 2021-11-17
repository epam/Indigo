#pragma once

#include <memory>

#include <pcg_random.hpp>

namespace indigo
{
    class Random
    {
    private:
        std::unique_ptr<pcg32_fast> _rng;

    public:
        Random() = delete;
        explicit Random(int seed);

        int next();
        int next(int mod);

        double nextDouble();
    };
}
