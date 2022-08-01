import errno
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("ignore-bad-valence", "true")


mol = indigo.loadMolecule("n1cc-2c[nH]n(c2n1)C")
print(mol.smiles())
mol.dearomatize()
s = ""
for a in mol.iterateAtoms():
    s = s + " " + str(a.countImplicitHydrogens())
print(s)


mol = indigo.loadMolecule("c1ccc2-c(nnc2)o1")
print(mol.smiles())
mol.dearomatize()
s = ""
for a in mol.iterateAtoms():
    s = s + " " + str(a.countImplicitHydrogens())
print(s)


mol = indigo.loadMolecule("n1c2-c(cnn2)cc[nH]1")
print(mol.smiles())
mol.dearomatize()
s = ""
for a in mol.iterateAtoms():
    s = s + " " + str(a.countImplicitHydrogens())
print(s)


mol = indigo.loadMolecule("c-12ccc[nH]c1nnn2")
print(mol.smiles())
mol.dearomatize()
s = ""
for a in mol.iterateAtoms():
    s = s + " " + str(a.countImplicitHydrogens())
print(s)
