#include <gtest/gtest.h>

#include <IndigoMolecule.h>
#include <IndigoRenderer.h>
#include <IndigoSession.h>
#include <thread>
#include <random>

using namespace indigo_cpp;

namespace
{
    static std::string choices[] = {"C", "CC", "CCC", "CCCC", "CCCCC", "CCCCCC"};
    static std::random_device rd;     // only used once to initialise (seed) engine
    static std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
    static std::uniform_int_distribution<int> uni(0,5);

    std::string randomSmiles()
    {
        return choices[uni(rng)];
    }

    void testRender()
    {
        const auto& session = IndigoSession();
        const auto& renderer = IndigoRenderer(session);
        const auto& smiles = randomSmiles();
        const auto& m = session.loadMolecule(smiles);
        const auto& result_1 = renderer.png(m);
        const auto& result_2 = renderer.png(m);
        ASSERT_EQ(result_1, result_2);
    }
}

TEST(RenderingThreads, Basic)
{
    std::vector<std::thread> threads;
    threads.reserve(1000);
    for (auto i = 0; i < 1000; i++)
    {
        threads.emplace_back(testRender);
    }
    for (auto& thread : threads)
    {
        thread.join();
    }
}
