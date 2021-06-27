#include <fstream>

#include <gtest/gtest.h>

#include <indigo.h>

#include "common.h"

using namespace indigo;

void prepareStructure(int mol)
{
    int atoms_it = indigoIterateAtoms(mol);
    while (indigoHasNext(atoms_it))
    {
        int atom = indigoNext(atoms_it);
        indigoSetXYZ(atom, 0, 0, 0);
    }

	indigoFree(atoms_it);

    int rgroups_it = indigoIterateRGroups(mol);
    while(indigoHasNext(rgroups_it))
    {
        int rg = indigoNext(rgroups_it); // take rgroup
        int fragments_it = indigoIterateRGroupFragments(rg);
        if (indigoHasNext(fragments_it))
        {
            int rg_next = indigoNext(fragments_it); // take first fragment.
            int atoms_it = indigoIterateAtoms(rg_next);
            while (indigoHasNext(atoms_it))
            {
                int atom = indigoNext(atoms_it);
                indigoSetXYZ(atom, 0, 0, 0);
            }
            indigoFree( atoms_it );
        }
        indigoFree( fragments_it );
    }
	indigoFree(rgroups_it);
}

void testScaffold(const std::string& filename, const std::string& mode, bool print_molfile)
{
    indigoSetOptionBool("molfile-saving-skip-date", true);
    indigoSetOptionBool("treat-x-as-pseudoatom", true);
    indigoSetOptionBool("ignore-stereochemistry-errors", true);
    int arr = indigoCreateArray();
    int sdf_it = indigoIterateSDFile(filename.c_str());
    while (indigoHasNext(sdf_it))
    {
        int item = indigoNext( sdf_it ); // take molecule
        indigoClearStereocenters(item);
        indigoClearCisTrans(item);
        indigoArrayAdd(arr, item);
    }
    indigoFree(sdf_it);
    int scaf = indigoExtractCommonScaffold(arr, mode.c_str());
    prepareStructure(scaf);
    std::cout << "scaffold:" << indigoSmiles(scaf) << std::endl;
    int all_scaffolds = indigoAllScaffolds( scaf );
    int deco = indigoDecomposeMolecules( scaf, arr );
    int full_scaf = indigoDecomposedMoleculeScaffold(deco);
    std::cout << "full scaffold: " << indigoSmiles(full_scaf) << std::endl;
    if (print_molfile)
    {
        prepareStructure(full_scaf);
        std::cout << "full scaffold: " << indigoMolfile( full_scaf ) << std::endl;
    }

    int decos_it = indigoIterateDecomposedMolecules(deco);
    while (indigoHasNext(decos_it))
    {
        int item = indigoNext(decos_it);
		std::cout << indigoSmiles( indigoDecomposedMoleculeHighlighted(item) ) << std::endl;
        int mol = indigoDecomposedMoleculeWithRGroups(item);
        std::cout << "decomposed molecule: " << indigoCanonicalSmiles(mol) << std::endl;
        if ( print_molfile )
	    {
            prepareStructure( mol );
            std::cout << "decomposed molecule: " << indigoMolfile(mol) << std::endl;
		}
        std::cout << "mapped scaffold: " << indigoCanonicalSmiles(indigoDecomposedMoleculeScaffold(item)) << std::endl;
        int rgroups = indigoIterateRGroups(mol);
		while (indigoHasNext(rgroups))
		{
            int rg = indigoNext( rgroups);
            std::cout << "  RGROUP #" << indigoIndex(rg) << std::endl;
            int frags = indigoIterateRGroupFragments( rg );
			if (indigoHasNext(frags))
			{
                int frag = indigoNext(frags);
                std::cout << "  fragment #" << indigoIndex(frag) << ":" << indigoCanonicalSmiles(frag) << std::endl;
            }
            else
                std::cout << "NO FRAGMENT" << std::endl;
		}
    }
    sdf_it = indigoIterateSDFile(filename.c_str());
    while (indigoHasNext(sdf_it))
    {
        int item = indigoNext(sdf_it);
        int sm = indigoSubstructureMatcher( item, nullptr );
		if (!indigoMatch(sm, full_scaf))
		{
            std::cout << "ERROR: full scaffold not found in the input structure " << indigoIndex(item) << std::endl;
		}
        int scaffs_it = indigoIterateArray(all_scaffolds);
        while (indigoHasNext(scaffs_it))
		{
            int scaf = indigoNext(scaffs_it);
            int sm = indigoSubstructureMatcher(item, nullptr);
            if (!indigoMatch(sm, scaf))
            {
                std::cout << "ERROR: scaffold " << indigoIndex( scaf ) << " not found in the input structure " << indigoIndex( item ) << std::endl;
            }
		}
    }
}

TEST(IndigoDecoSDFTest, deco_sdf_test)
{
    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);

    indigoSetErrorHandler(errorHandling, 0);

    try
    {
        testScaffold(dataPath("molecules/basic/thiazolidines.sdf"), "exact 10000", false);
        testScaffold(dataPath("molecules/basic/thiazolidines.sdf"), "approx", false);
        testScaffold(dataPath("molecules/basic/thiazolidines.sdf"), "approx 3", false);
        testScaffold(dataPath("molecules/basic/sugars.sdf"), "exact", true);
        testScaffold(dataPath("molecules/basic/sugars.sdf"), "approx", false);
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}
