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
from env_indigo import Indigo, joinPathPy  # noqa

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)
indigo.setOption("ignore-stereochemistry-errors", True)
indigo.setOption("molfile-saving-skip-date", True)

print("*** 2713 star atom support ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

filename = "star_atom"
expected_smiles = "*C.*N.*O |$;;AH_p;;star_e;$|"

mol = indigo.loadQueryMolecule(expected_smiles)

# with open(os.path.join(ref_path, filename) + ".ket", "w") as file:
#     file.write(mol.json())
with open(os.path.join(ref_path, filename) + ".ket", "r") as file:
    ket_ref = file.read()
ket = mol.json()
diff = find_diff(ket_ref, ket)
if not diff:
    print(filename + " ket : SUCCEED")
else:
    print(filename + " ket : FAILED")
    print(diff)
indigo.setOption("smiles-saving-format", "chemaxon")
smiles3 = mol.smiles()
diff = find_diff(smiles3, smiles3)
if not diff:
    print(filename + " scxmiles : SUCCEED")
else:
    print(filename + " scxmiles : FAILED")
    print(diff)
