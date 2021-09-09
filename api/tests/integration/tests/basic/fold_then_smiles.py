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

foldThenSmiles(joinPathPy('molecules/li-h.mol', __file__))
foldThenSmiles(joinPathPy('molecules/pc-438107.mol', __file__))
foldThenSmiles(joinPathPy('molecules/pc-20749491.mol', __file__))
