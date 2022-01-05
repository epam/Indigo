import sys

sys.path.append("../../common")
from env_indigo import *


#
# Prepare a molecule for printing out
#
def prepareStructure(mol):
    for atom in mol.iterateAtoms():
        atom.setXYZ(0, 0, 0)

    for rg in mol.iterateRGroups():
        if rg.iterateRGroupFragments().hasNext():
            rg_next = rg.iterateRGroupFragments().next()
            for atom in rg_next.iterateAtoms():
                atom.setXYZ(0, 0, 0)


indigo = Indigo()


def testScaffold(filename, mode, print_molfile):
    indigo.setOption("molfile-saving-skip-date", True)
    indigo.setOption("treat-x-as-pseudoatom", True)
    indigo.setOption("ignore-stereochemistry-errors", True)
    arr = indigo.createArray()
    for item in indigo.iterateSDFile(filename):
        item.clearStereocenters()
        item.clearCisTrans()
        #        item.aromatize()
        #        for atom in item.iterateAtoms():
        #            atom.setXYZ(0, 0, 0)
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
        print(
            "mapped scaffold: "
            + item.decomposedMoleculeScaffold().canonicalSmiles()
        )
        for rg in mol.iterateRGroups():
            print("  RGROUP #%d" % ((rg.index())))
            if rg.iterateRGroupFragments().hasNext():
                frag = rg.iterateRGroupFragments().next()
                print(
                    "  fragment #%s: %s"
                    % (str(frag.index()), frag.canonicalSmiles())
                )
            else:
                print("NO FRAGMENT")
    for item in indigo.iterateSDFile(filename):
        if not indigo.substructureMatcher(item).match(full_scaf):
            print(
                "ERROR: full scaffold not found in the input structure "
                + item.index()
            )
        for scaf in all_scaffolds.iterateArray():
            if not indigo.substructureMatcher(item).match(scaf):
                print(
                    "ERROR: scaffold "
                    + scaf.index()
                    + " not found in the input structure "
                    + item.index()
                )


try:
    testScaffold(
        joinPathPy(
            "../../../../../data/molecules/basic/thiazolidines.sdf", __file__
        ),
        "exact 5",
        False,
    )
except IndigoException as e:
    print("caught " + getIndigoExceptionText(e))

testScaffold(
    joinPathPy(
        "../../../../../data/molecules/basic/thiazolidines.sdf", __file__
    ),
    "exact 10000",
    False,
)
testScaffold(
    joinPathPy(
        "../../../../../data/molecules/basic/thiazolidines.sdf", __file__
    ),
    "approx",
    False,
)
testScaffold(
    joinPathPy(
        "../../../../../data/molecules/basic/thiazolidines.sdf", __file__
    ),
    "approx 3",
    False,
)
testScaffold(
    joinPathPy("../../../../../data/molecules/basic/sugars.sdf", __file__),
    "exact",
    True,
)
testScaffold(
    joinPathPy("../../../../../data/molecules/basic/sugars.sdf", __file__),
    "approx",
    False,
)
