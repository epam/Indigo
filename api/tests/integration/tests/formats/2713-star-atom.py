import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
from env_indigo import Indigo, joinPathPy  # noqa

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)
indigo.setOption("ignore-stereochemistry-errors", True)
indigo.setOption("molfile-saving-skip-date", True)

print("*** 2713 star atom support ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

filename = "star_atom" + ".ket"
expected_smiles = "*C.*N.*O |$;;AH_p;;star_e;$|"

mol = indigo.loadMolecule(expected_smiles)
ket = mol.json()
compare_diff(ref_path, filename, ket)

indigo.setOption("smiles-saving-format", "chemaxon")
smiles3 = mol.smiles()
print(smiles3)
