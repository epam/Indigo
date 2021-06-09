#include <fstream>

#include <gtest/gtest.h>

#include <base_cpp/cancellation_handler.h>
#include <base_cpp/scanner.h>
#include <molecule/max_common_submolecule.h>

#include "common.h"

using namespace indigo;

namespace
{
    static bool hasSolution(const ArrayNew<int>& map)
    {
        bool result = false;
        for (int i = 0; i < map.size(); ++i)
        {
            if (map[i] >= 0)
                result = true;
        }
        return result;
    }

    static int mapSize(const ArrayNew<int>& map)
    {
        int result = 0;
        for (int i = 0; i < map.size(); ++i)
        {
            if (map[i] >= 0)
                ++result;
        }
        return result;
    }
}

TEST(IndigoMcsBasicTest, mcs_one_atom)
{
    Molecule t_mol;
    Molecule q_mol;

    loadMolecule("C", t_mol);
    loadMolecule("CCO", q_mol);

    MaxCommonSubmolecule mcs(t_mol, q_mol);

    mcs.findExactMCS();
    ArrayNew<int> v_map;
    ArrayNew<int> e_map;
    mcs.getMaxSolutionMap(&v_map, &e_map);

    ASSERT_EQ(1, mcs.parametersForExact.numberOfSolutions);
    ASSERT_EQ(1, v_map.size());
    ASSERT_EQ(0, v_map[0]);
}

TEST(IndigoMcsBasicTest, mcs_find_2_atom_mcs)
{
    Molecule t_mol;
    Molecule q_mol;

    loadMolecule("CCO", q_mol);
    loadMolecule("CCC", t_mol);

    MaxCommonSubmolecule mcs(t_mol, q_mol);

    mcs.findExactMCS();
    ArrayNew<int> v_map;
    ArrayNew<int> e_map;
    mcs.getMaxSolutionMap(&v_map, &e_map);
    ASSERT_EQ(1, mcs.parametersForExact.numberOfSolutions);
    ASSERT_TRUE(hasSolution(e_map));
}

TEST(IndigoMcsBasicTest, mcs_find_2_atom_mcs_with_input_map)
{
    Molecule t_mol;
    Molecule q_mol;

    loadMolecule("CCO", q_mol);
    loadMolecule("CCC", t_mol);

    MaxCommonSubmolecule mcs(q_mol, t_mol);
    mcs.incomingMap.resize(3);
    mcs.incomingMap.fill(-1);
    mcs.incomingMap[0] = 0;

    mcs.findExactMCS();
    ArrayNew<int> v_map;
    ArrayNew<int> e_map;
    mcs.getMaxSolutionMap(&v_map, &e_map);
    ASSERT_EQ(1, mcs.parametersForExact.numberOfSolutions);
    ASSERT_EQ(0, v_map[0]);
    ASSERT_EQ(1, v_map[1]);
}

TEST(IndigoMcsBasicTest, mcs_find_2_atom_and_edges)
{
    Molecule t_mol;
    Molecule q_mol;

    loadMolecule("C=CO", q_mol);
    loadMolecule("CCC", t_mol);

    MaxCommonSubmolecule mcs(q_mol, t_mol);
    mcs.incomingMap.resize(3);
    mcs.incomingMap.fill(-1);
    mcs.incomingMap[0] = 0;

    mcs.findExactMCS();
    ArrayNew<int> v_map;
    ArrayNew<int> e_map;
    mcs.getMaxSolutionMap(&v_map, &e_map);
    ASSERT_EQ(0, mcs.parametersForExact.numberOfSolutions);
}

TEST(IndigoMcsBasicTest, DISABLED_mcs_finish_on_timeout)
{
    Molecule t_mol;
    Molecule q_mol;

    loadMolecule("CC1CC2CCC3CC4=CC=C5CCCC6=C/C=C7/C8CCCC9CC%10CCCC%11CC%12=C(C(C%10%11)C89)C7=CC(C2=C3CC4=C56)=C1%12", q_mol);
    loadMolecule("C1CC2CC3CCCC4C3C3C2C(C1)C1CCC2CC5CCC6CC7=CC=C8CCCC9=C%10C=C4C4=C%11C(C5=C6C(C7=C89)=C%10%11)=C2C1=C34", t_mol);

    MaxCommonSubmolecule mcs(q_mol, t_mol);

    try
    {
        resetCancellationHandler(new TimeoutCancellationHandler(500));
        mcs.findExactMCS();
        ASSERT_TRUE(false);
    }
    catch (Exception& e)
    {
        //      ASSERT_STREQ("", e.message());
    }
    //         ArrayNew<int> v_map;
    //         ArrayNew<int> e_map;
    //         mcs.getMaxSolutionMap(&v_map, &e_map);

    ASSERT_EQ(0, mcs.parametersForExact.numberOfSolutions);
}

TEST(IndigoMcsBasicTest, mcs_test_rings)
{
    Molecule t_mol;
    Molecule q_mol;

    resetCancellationHandler(nullptr);

    std::fstream flog;
    flog.open("mcs_test.log", std::fstream::out);

    loadMolecule("C1C(=CC(=CC=1C1C=CC=CC=1)C1C=CC=CC=1)C1C=CC=CC=1", q_mol);
    loadMolecule("C1C(=CC=CC=1C1C=CC=CC=1C1C=CC=CC=1)C1C=CC=CC=1", t_mol);

    MaxCommonSubmolecule mcs(t_mol, q_mol);

    mcs.findExactMCS();
    ArrayNew<int> v_map;
    ArrayNew<int> e_map;
    mcs.getMaxSolutionMap(&v_map, &e_map);
    flog << "Size" << v_map.size() << "\n";
    for (int i = 0; i < v_map.size(); ++i)
    {
        flog << "" << i << ": " << v_map[i] << "\n";
    }
    flog.flush();

    ASSERT_EQ(18, mapSize(v_map));
}