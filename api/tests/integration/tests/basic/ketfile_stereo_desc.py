import difflib
import os
import sys


def find_diff(a, b):
    return "\n".join(difflib.unified_diff(a.splitlines(), b.splitlines()))


sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)

from env_indigo import *

ref_path = joinPathPy("ref/", __file__)
root = joinPathPy("molecules/CIP/", __file__)

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)

for filename in sorted(os.listdir(root)):
    ketfile = joinPathPy(
        os.path.join(ref_path, filename[:-4] + ".ket"), __file__
    )
    mol = indigo.loadMoleculeFromFile(os.path.join(root, filename))
    with open(ketfile, "r") as file:
        ket_ref = file.read()
    mol_json_no_cip = mol.json()
    indigo.setOption("json-saving-add-stereo-desc", True)
    mol_json_cip = mol.json()
    diff = find_diff(ket_ref, mol_json_cip)
    if not diff:
        diff = find_diff(mol_json_no_cip, mol_json_cip)
        indigo.setOption("json-saving-add-stereo-desc", False)
        mol = indigo.loadMoleculeFromFile(
            os.path.join(root, filename)
        )  # reload to reset CIP
        if diff:
            diff = find_diff(mol.json(), mol_json_no_cip)
            if diff:
                print("mismatch: json-saving-add-stereo-desc = false:")
            else:
                # check conversion
                indigo.setOption("molfile-saving-add-stereo-desc", True)
                mol_cip = indigo.loadMolecule(
                    mol.molfile()
                )  # mol_cip should contain CIP as properties of atoms and bonds
                diff = find_diff(
                    mol_cip.json(), mol_json_cip
                )  # check if molecule has CIP
                if diff:
                    print(
                        "mismatch: molfile loader doesn't convert CIP SGroups:"
                    )
                else:
                    print(filename + ":SUCCEED")
                    continue
        print(filename + ":FAILED")
        print(diff)

indigo.setOption("ignore-stereochemistry-errors", "true")
filename = "crazystereo.rxn"

rxn = indigo.loadReactionFromFile(
    os.path.join(joinPathPy("reactions/", __file__), filename)
)

indigo.setOption("json-use-native-precision", True)
ketfile = joinPathPy(os.path.join(ref_path, "crazystereo.ket"), __file__)
# with open(ketfile, "w") as file:
#     file.write(rxn.json())
with open(ketfile, "r") as file:
    ket_ref = file.read()
    diff = find_diff(ket_ref, rxn.json())
    if not diff:
        print(filename + ":SUCCEED")
    else:
        print(filename + ":FAILED")
        print(diff)
