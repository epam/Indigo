#include <fstream>

#include <gtest/gtest.h>

#include <indigo.h>

#include "common.h"

using namespace indigo;

void prepareStructure(int mol)
{
    int atoms = indigoIterateAtoms(mol);
    while (indigoHasNext(atoms))
    {
        int atom = indigoNext(atoms);
        indigoSetXYZ(atom, 0, 0, 0);
    }

    int rgroups = indigoIterateRGroups(mol);
    while (indigoHasNext(rgroups))
    {
        int rg = indigoNext(rgroups);
        int fragments = indigoIterateRGroupFragments(rg);
        if (indigoHasNext(fragments))
        {
            int rg_next = indigoNext(fragments);
            int atoms = indigoIterateAtoms(rg_next);
            while (indigoHasNext(atoms))
            {
                int atom = indigoNext(atoms);
                indigoSetXYZ(atom, 0, 0, 0);
            }
        }
    }
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
        int item = indigoNext( sdf_it );
        indigoClearStereocenters(item);
        indigoClearCisTrans(item);
        indigoArrayAdd(arr, item);
    }
    int scaf = indigoExtractCommonScaffold(arr, mode.c_str());
    prepareStructure(scaf);
    std::cout << "scaffold:" << indigoSmiles(scaf) << std::endl;
    int all_scaffolds = indigoAllScaffolds( scaf );
    int deco = indigoDecomposeMolecules( scaf, arr );
    int full_scaf = indigoDecomposedMoleculeScaffold(deco);
    std::cout << "full scaffold:" << indigoSmiles(full_scaf) << std::endl;
    if (print_molfile)
    {
        prepareStructure(full_scaf);
        std::cout << "full scaffold:" << indigoMolfile( full_scaf ) << std::endl;
    }

    int decos = indigoIterateDecomposedMolecules(deco);
    while (indigoHasNext(decos))
    {
        int item = indigoNext(decos);

    }
}

    /*def testScaffold(filename, mode, print_molfile):
        indigo.setOption("molfile-saving-skip-date", True)
        indigo.setOption("treat-x-as-pseudoatom", True)
        indigo.setOption("ignore-stereochemistry-errors", True)
        arr = indigo.createArray()
        for item in indigo.iterateSDFile(filename):
            item.clearStereocenters()
            item.clearCisTrans()
    #item.aromatize()
    #for atom in item.iterateAtoms():
    #atom.setXYZ(0, 0, 0)
            arr.arrayAdd(item)

        scaf = indigo.extractCommonScaffold(arr, mode)
        prepareStructure(scaf)
        print("scaffold: " + scaf.smiles())
        all_scaffolds = scaf.allScaffolds()
        deco = indigo.decomposeMolecules(scaf, arr)
        full_scaf = deco.decomposedMoleculeScaffold()
        print("full scaffold: " + full_scaf.smiles())

        if print_molfile:
            prepareStructure(full_scaf)
            print("full scaffold: " + full_scaf.molfile())
        for item in deco.iterateDecomposedMolecules():
            print(item.decomposedMoleculeHighlighted().smiles())
            mol = item.decomposedMoleculeWithRGroups()
            print("decomposed molecule: " + mol.canonicalSmiles())
            if print_molfile:
                prepareStructure(mol)
                print("decomposed molecule: " + mol.molfile())
            print("mapped scaffold: " + item.decomposedMoleculeScaffold().canonicalSmiles())
            for rg in mol.iterateRGroups():
                print("  RGROUP #%d" % ((rg.index())))
                if rg.iterateRGroupFragments().hasNext():
                    frag = rg.iterateRGroupFragments().next()
                    print("  fragment #%s: %s" % (str(frag.index()), frag.canonicalSmiles()))
                else:
                    print("NO FRAGMENT")
        for item in indigo.iterateSDFile(filename):
            if not indigo.substructureMatcher(item).match(full_scaf):
                print("ERROR: full scaffold not found in the input structure " + item.index())
            for scaf in all_scaffolds.iterateArray():
                if not indigo.substructureMatcher(item).match(scaf):
                    print("ERROR: scaffold " + scaf.index() + " not found in the input structure " + item.index())
    */

    TEST(IndigoDecoSDFTest, deco_sdf_test)
{
    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);

    indigoSetErrorHandler(errorHandling, 0);

    try
    {
        int m = indigoLoadMoleculeFromString("c1ccccc1.c1ccccc1");
        int c = indigoComponent(m, 0);
        int cc = indigoClone(c);
        indigoDearomatize(cc);
        Array<int> vertices;
        for (int i = 0; i < 6; ++i)
            vertices.push(i);

        indigoRemoveAtoms(m, vertices.size(), vertices.ptr());
        //      printf("%s\n", indigoSmiles(cc));

        indigoMerge(m, cc);
        ASSERT_STREQ("C1C=CC=CC=1.c1ccccc1", indigoSmiles(m));
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}
