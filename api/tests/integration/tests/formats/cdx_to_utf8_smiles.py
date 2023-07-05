import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

indigo = Indigo()

root = joinPathPy("molecules/cdx", __file__)
mol = indigo.loadMoleculeFromFile(os.path.join(root, "invalid-ascii.cdx"))
print(mol.smiles())