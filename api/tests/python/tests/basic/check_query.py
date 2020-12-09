import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

indigo = Indigo()

indigo.setOption("molfile-saving-skip-date", True)
indigo.setOption("ignore-noncritical-query-features", "true")

mol = indigo.loadMoleculeFromFile(joinPath("molecules/Query_for_check.mol"))

print(mol.molfile())

for atom in mol.iterateAtoms():
    print("Check query for atom %d returns %d" % (atom.index(), atom.checkQuery()))

for bond in mol.iterateBonds():
    print("Check query for bond %d returns %d" % (bond.index(), bond.checkQuery()))


mol = indigo.loadQueryMoleculeFromFile(joinPath("molecules/Query_for_check.mol"))

print(mol.molfile())

for atom in mol.iterateAtoms():
    print("Check query for atom %d returns %d" % (atom.index(), atom.checkQuery()))

for bond in mol.iterateBonds():
    print("Check query for bond %d returns %d" % (bond.index(), bond.checkQuery()))


mol = indigo.loadMolecule("c1[n]c2c(N)[n+]([O-])c[n]c2[n]1[C@H]1[C@@H](O)[C@H](O)[C@H](CO)O1")

print(mol.smiles())
print(mol.molfile())

for atom in mol.iterateAtoms():
    print("Check query for atom %d returns %d" % (atom.index(), atom.checkQuery()))

for bond in mol.iterateBonds():
    print("Check query for bond %d returns %d" % (bond.index(), bond.checkQuery()))
