#ifndef _RANDOM_H_
#define _RANDOM_H_
namespace indigo
{

    class Random
    {
    private:
        unsigned long long randSeed;

    public:
        Random();
        Random(int seed);

        void setSeed(unsigned long long x);

        unsigned int next();
        unsigned int next(int mod);
        unsigned int nextBounded(int l, int r);
        unsigned int nextLarge(int mod);

        unsigned long long nextLong();
        unsigned long long nextLong(unsigned long long mod);

        double nextDouble();
        double nextDoubleBounded(double l, double r);
    };

} // namespace indigo

#endif