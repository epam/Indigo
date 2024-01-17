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


def test_qmol_unfold(filename):
    print("\ntesting filename:\n%s" % filename)
    molecule = indigo.loadQueryMoleculeFromFile(ket_path + filename)
    molecule.unfoldHydrogens()
    unfolded = molecule.json()
    # with open(ref_path + filename, "w") as ref_file:
    #    ref_file.write(unfolded)
    with open(ref_path + filename) as ref_file:
        ref_json = ref_file.read()
    unfolded_list = sorted(unfolded.split("\n"))
    expected_unfolded_list = sorted(ref_json.split("\n"))
    diff = "".join(difflib.context_diff(unfolded_list, expected_unfolded_list))
    if diff:
        print("\nDiff between expected and after unfold molfile:\n%s" % diff)
    else:
        print("Unfolded KET equal to expected")


print("******* Test unfold aromatic ring *******")
test_qmol_unfold("issue_1525.ket")
