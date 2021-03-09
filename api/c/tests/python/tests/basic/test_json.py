import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")

mol = indigo.loadMoleculeFromFile(joinPath("molecules/test_json.json"))
print(mol.molfile())


mol = indigo.loadMolecule("Nc1[nH]cnc-2ncnc1-2")
s = mol.json()
print(s)
print(mol.molfile())

mol = indigo.loadMolecule(s)
print(mol.molfile())

