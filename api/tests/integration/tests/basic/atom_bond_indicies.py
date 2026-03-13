import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import Indigo
indigo = Indigo()

print("issue 2952 expand c api for cip labels")
mol = indigo.loadMolecule("C[C@@H](F)C[C@H](C)Cl")
for atom in mol.iterateAtoms():
    print("atom index: " + str(atom.atomIndex()))
for bond in mol.iterateBonds():
    print(
        "bond index, begining and ending atoms: "
        + str(bond.bondIndex())
        + " "
        + str(bond.bondBegin())
        + " "
        + str(bond.bondEnd())
    )
