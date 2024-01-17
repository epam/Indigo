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
    out_path = joinPathPy("out", __file__) + "/"
    with open(out_path + filename, "w") as out_file:
        out_file.write(unfolded)
    with open(ref_path + filename) as ref_file:
        ref_json = ref_file.read()
    unfolded_list = unfolded.split("\n")
    expected_list = ref_json.split("\n")
    diff = "\n".join(difflib.context_diff(unfolded_list, expected_list))
    if diff:
        print("Diff between expected and after unfold:\n%s" % diff)
    else:
        print("Unfolded KET equal to expected")


print("******* Test unfold aromatic ring *******")
test_qmol_unfold("issue_1525.ket")
