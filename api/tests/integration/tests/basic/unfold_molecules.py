from __future__ import print_function

import difflib
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import Indigo, joinPathPy

ket_path = joinPathPy("molecules", __file__) + "/"
ref_path = joinPathPy("ref", __file__) + "/"

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")
indigo.setOption("json-saving-pretty", "1")
indigo.setOption("json-use-native-precision", "1")


def test_unfold(filename, load_function, auto=False):
    print("testing filename:\n%s" % filename)
    molecule = load_function(ket_path + filename)
    init_smarts = molecule.smarts()
    if auto:
        molecule.foldUnfoldHydrogens()
    else:
        molecule.unfoldHydrogens()
    unfolded = molecule.smarts()
    # out_path = joinPathPy("out", __file__) + "/"
    # with open(out_path + filename, "w") as out_file:
    #     out_file.write(molecule.json())
    expected_mol = load_function(ref_path + filename)
    expected = expected_mol.smarts()
    if expected == unfolded:
        print("Unfolded molecule equal to expected")
    else:
        print(
            "Diff between expected and after unfold.\nExpected:%s\nUnfolded:%s\n"
            % (expected, unfolded)
        )
    if auto:
        molecule.foldUnfoldHydrogens()
    else:
        molecule.foldHydrogens()
    folded = molecule.smarts()
    if folded == init_smarts:
        print("Folded KET equal to original")
    else:
        print(
            "Diff between origin and after fold.\nOrigin:%s\nFolded:%s\n"
            % (expected, unfolded)
        )


def test_qmol_unfold(filename, auto=False):
    test_unfold(filename, indigo.loadQueryMoleculeFromFile, auto)


def test_mol_unfold(filename, auto=False):
    test_unfold(filename, indigo.loadMoleculeFromFile, auto)


print("\n******* Test unfold aromatic ring *******")
test_qmol_unfold("issue_1525.ket")

print("\n******* Test unfold fullerene *******")
test_qmol_unfold("issue_1573.ket")

print("\n******* Test unfold fullerene with second component *******")
test_qmol_unfold("issue_1573_2.ket")

print("\n******* Test unfold selected atoms *******")
test_mol_unfold("issue_1589.ket")

print("\n******* Test unfold bad valence *******")
test_mol_unfold("issue_1538.ket")

print("\n******* Test unfold any atom with valence *******")
test_qmol_unfold("issue_1550.ket")

print("\n******* Test unfold selected with unselected explicit H *******")
test_mol_unfold("issue_1632.ket", auto=True)

print(
    "\n******* Test unfold two selected with one unselected molecules *******"
)
test_mol_unfold("issue_1640.ket")


print("\n******* Test unfold radicals in query molecules *******")
test_qmol_unfold("issue_1634.ket")
