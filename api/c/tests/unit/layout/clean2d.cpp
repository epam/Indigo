#include <fstream>
#include <utility>
#include <gtest/gtest.h>

#include <indigo.h>

#include "common.h"

using namespace indigo;


void get_coordinates(int mol, std::vector<int>& atoms, std::vector<std::pair<double,double>>& coords )
{
    int it = indigoIterateAtoms(mol);
    while (indigoHasNext(it))
    {
        int atom = indigoNext(it);
        float* xyz = indigoXYZ(atom);
        atoms.push_back(indigoAtomicNumber(atom));
        coords.emplace_back(xyz[0], xyz[1]);
    }
}

void print_compare(int mol, int mol_ref )
{
    std::vector<int> atoms1, atoms2;
    std::vector<std::pair<double, double>> coords1, coords2;
    get_coordinates( mol, atoms1, coords1 );
    get_coordinates( mol_ref, atoms2, coords2);
    for (int i = 0; i < atoms1.size(); ++i)
    {
        printf("atom=%d; (%f,%f)-(%f,%f) \n",atoms1[i], coords1[i].first, coords1[i].second, coords2[i].first, coords2[i].second );
    }
}


TEST(IndigoClean2DTest, clean2d_test)
{
    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);
    indigoSetErrorHandler(errorHandling, 0);
    indigoSetOptionBool("molfile-saving-skip-date", true);
    indigoSetOptionBool("treat-x-as-pseudoatom", true);
    try
    {
        int sdf_org_it = indigoIterateSDFile(dataPath("molecules/layout/clean2d.sdf").c_str());
        int sdf_ref_it = indigoIterateSDFile(dataPath("molecules/layout/clean2dref.sdf").c_str());
        while (indigoHasNext(sdf_org_it) && indigoHasNext(sdf_ref_it))
        {
            int item_org = indigoNext(sdf_org_it); // take molecule
            int item_ref = indigoNext(sdf_ref_it);
            int cl2d = indigoClean2d(item_org);
            print_compare(item_org, item_ref);
            printf("------------------>\n");
        }
        indigoFree(sdf_org_it);
        indigoFree(sdf_ref_it);
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}

