#include <gtest/gtest.h>

#include <random>
#include <thread>

#include <IndigoMolecule.h>
#include <IndigoRenderer.h>
#include <IndigoSession.h>

using namespace indigo_cpp;

namespace
{
    static std::string choices[] = {"C", "CC", "CCC", "CCCC", "CCCCC", "CCCCCC"};
    static std::random_device rd;
    static std::mt19937 rng(rd());
    static std::uniform_int_distribution<int> uni(0, 6);

    std::string randomSmiles()
    {
        return choices[uni(rng)];
    }

    void testCanonicalSmiles()
    {
        const auto& smiles = randomSmiles();
        const auto& session_1 = IndigoSession();
        const auto& session_2 = IndigoSession();
        const auto& m_1 = session_1.loadMolecule(smiles);
        const auto& m_2 = session_2.loadMolecule(smiles);
        const auto& result_1 = m_1.smiles();
        const auto& result_2 = m_2.smiles();
        ASSERT_EQ(result_1, result_2);
    }
} // namespace

TEST(BasicThreads, Basic)
{
    std::vector<std::thread> threads;
    threads.reserve(100);
    for (auto i = 0; i < 100; i++)
    {
        threads.emplace_back(testCanonicalSmiles);
    }
    for (auto& thread : threads)
    {
        thread.join();
    }
}
