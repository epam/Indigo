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
    ket_json = molecule.json()
    ket_list = ket_json.split("\n")
    if auto:
        molecule.foldUnfoldHydrogens()
    else:
        molecule.unfoldHydrogens()
    unfolded = molecule.json()
    unfolded_list = unfolded.split("\n")
    # out_path = joinPathPy("out", __file__) + "/"
    # with open(out_path + filename, "w") as out_file:
    #     out_file.write(unfolded)
    with open(ref_path + filename) as ref_file:
        ref_json = ref_file.read()
    expected_list = ref_json.split("\n")
    diff = "\n".join(difflib.context_diff(unfolded_list, expected_list))
    if diff:
        print("Diff between expected and after unfold:\n%s" % diff)
    else:
        print("Unfolded KET equal to expected")
    if auto:
        molecule.foldUnfoldHydrogens()
    else:
        molecule.foldHydrogens()
    folded = molecule.json()
    folded_list = folded.split("\n")
    diff = "\n".join(difflib.context_diff(folded_list, ket_list))
    if diff:
        print("Diff between original ket and after fold:\n%s" % diff)
    else:
        print("Folded KET equal to original")


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
