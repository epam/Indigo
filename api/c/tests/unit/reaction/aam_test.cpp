#include <fstream>

#include <gtest/gtest.h>

#include <base_cpp/exception.h>
#include <base_cpp/scanner.h>
#include <molecule/molecule.h>

#include <indigo.h>

#include "common.h"

using namespace indigo;

namespace
{
    int numUniqueMap(ArrayNew<int>& map)
    {
        std::set<int> unique;
        for (int i = 0; i < map.size(); ++i)
        {
            if (map[i] >= 0)
            {
                unique.insert(map[i]);
            }
        }
        return unique.size();
    }

    void setAtomIndeces(int rxn)
    {
        int mols = indigoIterateMolecules(rxn);
        while (indigoHasNext(mols))
        {
            int mol = indigoNext(mols);
            int atoms = indigoIterateAtoms(mol);
            while (indigoHasNext(atoms))
            {
                int atom = indigoNext(atoms);
                indigoSetAtomMappingNumber(rxn, atom, indigoIndex(atom));
            }
        }
    }
} // namespace

TEST(IndigoAAMTest, test_aam_rings)
{
    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);

    indigoSetErrorHandler(errorHandling, 0);

    //   indigoSetOption("render-coloring", "true");
    //   indigoSetOption("render-stereo-style", "none");
    //   indigoSetOption("render-image-size", "800, 800");
    //   indigoSetOption("render-output-format", "png");

    try
    {
        int rxn = indigoLoadReactionFromString("C1C(=CC(=CC=1C1C=CC=CC=1)C1C=CC=CC=1)C1C=CC=CC=1>>C1C(=CC=CC=1C1C=CC=CC=1C1C=CC=CC=1)C1C=CC=CC=1");

        indigoAutomap(rxn, "DISCARD");

        //      indigoRenderToFile(rxn, "test_aam.png");

        ArrayNew<int> map;
        int re = indigoIterateReactants(rxn);

        while (indigoHasNext(re))
        {
            int n = indigoNext(re);
            int mr = indigoIterateAtoms(n);
            while (indigoHasNext(mr))
            {
                int m = indigoNext(mr);
                map.push(indigoGetAtomMappingNumber(rxn, m));
            }
        }

        ASSERT_EQ(24, numUniqueMap(map));
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}

TEST(IndigoAAMTest, test_aam_alter)
{
    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);

    indigoSetErrorHandler(errorHandling, 0);

    try
    {
        int rxn = indigoLoadReactionFromString("C1CC[NH:2]CC1.C1CC[S:1]CC1>>C1CC2CC[S:2]CC2C[NH:1]1");

        indigoAutomap(rxn, "KEEP");

        //      indigoRenderToFile(rxn, "test_aam.png");

        ArrayNew<int> map;
        int re = indigoIterateReactants(rxn);

        while (indigoHasNext(re))
        {
            int n = indigoNext(re);
            int mr = indigoIterateAtoms(n);
            while (indigoHasNext(mr))
            {
                int m = indigoNext(mr);
                map.push(indigoGetAtomMappingNumber(rxn, m));
            }
        }

        ASSERT_EQ(11, numUniqueMap(map));
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}

TEST(IndigoAAMTest, test_aam_keep_radicals)
{
    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);

    indigoSetErrorHandler(errorHandling, 0);

    try
    {
        int rxn = indigoLoadReactionFromString("C[12CH2:1]C(CCCC)[CH]CCCCCCC>>C[13CH2:1]C(CCCC)[C]CCCCCCCC |^1:7,^4:22|");

        indigoAutomap(rxn, "KEEP");

        //      indigoRenderToFile(rxn, "test_aam.png");

        ArrayNew<int> map;
        int re = indigoIterateReactants(rxn);

        while (indigoHasNext(re))
        {
            int n = indigoNext(re);
            int mr = indigoIterateAtoms(n);
            while (indigoHasNext(mr))
            {
                int m = indigoNext(mr);
                map.push(indigoGetAtomMappingNumber(rxn, m));
            }
        }

        ASSERT_EQ(14, numUniqueMap(map));
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}

TEST(IndigoAAMTest, test_aam_395_1)
{
    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);

    indigoSetErrorHandler(errorHandling, 0);

    try
    {
        int rxn = indigoLoadReactionFromString("C1=CC=CC(O)=C1.CCCC>>C1=CC=CC=C1.CCCCO");

        indigoAutomap(rxn, "DISCARD");

        // indigoRenderToFile(rxn, "test_aam.png");

        ArrayNew<int> map;
        int re = indigoIterateReactants(rxn);

        while (indigoHasNext(re))
        {
            int n = indigoNext(re);
            int mr = indigoIterateAtoms(n);
            while (indigoHasNext(mr))
            {
                int m = indigoNext(mr);
                map.push(indigoGetAtomMappingNumber(rxn, m));
            }
        }

        ASSERT_EQ(11, numUniqueMap(map));
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}
