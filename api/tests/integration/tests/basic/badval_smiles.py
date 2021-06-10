import os
import sys

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))), 'common'))
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)

print("*** Load molecule with bad valence atoms from MOL file *** ")
mol = indigo.loadMoleculeFromFile(joinPath("molecules/issue_243.mol"))
print(mol.molfile())
print("*** SMILES for that molecule *** ")
print(mol.smiles())
print("*** Load molecule with bad valence atoms from SMILES *** ")
mol = indigo.loadMolecule(mol.smiles())
print(mol.molfile())
