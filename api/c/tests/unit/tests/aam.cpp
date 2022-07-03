/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#include <fstream>
#include <vector>

#include <gtest/gtest.h>

#include <indigo.h>

#include "common.h"

using namespace indigo;
using namespace std;

class IndigoApiAamTest : public IndigoApiTest
{
protected:
    int numUniqueMap(vector<int>& map)
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
};

TEST_F(IndigoApiAamTest, test_aam_rings)
{
    try
    {
        int rxn = indigoLoadReactionFromString("C1C(=CC(=CC=1C1C=CC=CC=1)C1C=CC=CC=1)C1C=CC=CC=1>>C1C(=CC=CC=1C1C=CC=CC=1C1C=CC=CC=1)C1C=CC=CC=1");

        indigoAutomap(rxn, "DISCARD");

        //      indigoRenderToFile(rxn, "test_aam.png");

        vector<int> map;
        int re = indigoIterateReactants(rxn);

        while (indigoHasNext(re))
        {
            int n = indigoNext(re);
            int mr = indigoIterateAtoms(n);
            while (indigoHasNext(mr))
            {
                int m = indigoNext(mr);
                map.push_back(indigoGetAtomMappingNumber(rxn, m));
            }
        }

        ASSERT_EQ(24, numUniqueMap(map));
    }
    catch (std::exception& e)
    {
        ASSERT_STREQ("", e.what());
    }
}

TEST_F(IndigoApiAamTest, test_aam_alter)
{
    try
    {
        int rxn = indigoLoadReactionFromString("C1CC[NH:2]CC1.C1CC[S:1]CC1>>C1CC2CC[S:2]CC2C[NH:1]1");

        indigoAutomap(rxn, "KEEP");

        //      indigoRenderToFile(rxn, "test_aam.png");

        vector<int> map;
        int re = indigoIterateReactants(rxn);

        while (indigoHasNext(re))
        {
            int n = indigoNext(re);
            int mr = indigoIterateAtoms(n);
            while (indigoHasNext(mr))
            {
                int m = indigoNext(mr);
                map.push_back(indigoGetAtomMappingNumber(rxn, m));
            }
        }

        ASSERT_EQ(11, numUniqueMap(map));
    }
    catch (std::exception& e)
    {
        ASSERT_STREQ("", e.what());
    }
}

TEST_F(IndigoApiAamTest, test_aam_keep_radicals)
{
    try
    {
        int rxn = indigoLoadReactionFromString("C[12CH2:1]C(CCCC)[CH]CCCCCCC>>C[13CH2:1]C(CCCC)[C]CCCCCCCC |^1:7,^4:22|");

        indigoAutomap(rxn, "KEEP");

        //      indigoRenderToFile(rxn, "test_aam.png");

        vector<int> map;
        int re = indigoIterateReactants(rxn);

        while (indigoHasNext(re))
        {
            int n = indigoNext(re);
            int mr = indigoIterateAtoms(n);
            while (indigoHasNext(mr))
            {
                int m = indigoNext(mr);
                map.push_back(indigoGetAtomMappingNumber(rxn, m));
            }
        }

        ASSERT_EQ(14, numUniqueMap(map));
    }
    catch (std::exception& e)
    {
        ASSERT_STREQ("", e.what());
    }
}

TEST_F(IndigoApiAamTest, test_aam_395_1)
{
    try
    {
        int rxn = indigoLoadReactionFromString("C1=CC=CC(O)=C1.CCCC>>C1=CC=CC=C1.CCCCO");

        indigoAutomap(rxn, "DISCARD");

        // indigoRenderToFile(rxn, "test_aam.png");

        vector<int> map;
        int re = indigoIterateReactants(rxn);

        while (indigoHasNext(re))
        {
            int n = indigoNext(re);
            int mr = indigoIterateAtoms(n);
            while (indigoHasNext(mr))
            {
                int m = indigoNext(mr);
                map.push_back(indigoGetAtomMappingNumber(rxn, m));
            }
        }

        ASSERT_EQ(11, numUniqueMap(map));
    }
    catch (std::exception& e)
    {
        ASSERT_STREQ("", e.what());
    }
}
