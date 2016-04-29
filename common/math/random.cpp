#include <math.h>
#include "base_c/defs.h"
#include "math/random.h"
#include <stdlib.h>
#include <stdio.h>

using namespace indigo;

Random::Random() 
{
   randSeed = rand();
}

Random::Random(int seed)
{
   randSeed = seed;
}

void Random::setSeed(long long x) 
{
   randSeed = x;
}

unsigned int Random::next() 
{
//   printf("%lld %lld \n", randSeed, 6364136223846793005LL);
   randSeed = 6364136223846793005LL * randSeed + 1;
//   printf("%lld %lld \n", randSeed, 6364136223846793005LL);
   return (unsigned int) (randSeed >> 16);
}

unsigned int Random::next(int mod) 
{
   if (mod > 0) return next() % mod;
   if (mod < 0) return - (next() % - mod);
   return 0;
}

unsigned int Random::nextBounded(int l, int r) 
{
   return __min(l, r) + next(abs((long)(r - l)));
}

unsigned int Random::nextLarge(int mod) 
{
   int x = next();
   if ((1LL << 32) - x > mod) return x % mod;

   int max = (1LL << 32) - (1LL << 32) % mod;
   if (x < max) return x % mod;
   return nextLarge(mod);
}

long long Random::nextLong() 
{
   return ((long long)next() << 32) + next();
}

long long Random::nextLong(long long mod) 
{
   return nextLong() % mod;
}

double Random::nextDouble() 
{
   return 1.0 * next() / (1LL << 32);
}

double Random::nextDoubleBounded(double l, double r) 
{
   return __min(l, r) + nextDouble() * abs((long)(r - l));
}
