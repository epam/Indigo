from indigo import *

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", True)
indigo.setOption("ignore-bad-valence", True)
level = indigo.buildPkaModel(10, 0.5, 'pkamodel.sdf')

mol = indigo.loadMolecule("C(=C=O)=C=O")


for atom in mol.iterateAtoms():
    print(atom.symbol())
    print(mol.getAcidPkaValue(atom, 5, 2))

