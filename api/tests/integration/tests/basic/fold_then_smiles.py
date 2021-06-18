import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()

def foldThenSmiles(filename):
    print(relativePath(filename))
    mol = indigo.loadMoleculeFromFile(filename)
    mol.foldHydrogens()
    print(mol.smiles())
    mol.aromatize()
    print(mol.canonicalSmiles() + '\n')

foldThenSmiles(joinPath('molecules/li-h.mol'))
foldThenSmiles(joinPath('molecules/pc-438107.mol'))
foldThenSmiles(joinPath('molecules/pc-20749491.mol'))
