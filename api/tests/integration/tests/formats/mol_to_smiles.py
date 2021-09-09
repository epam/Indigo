import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
print(indigo.loadMoleculeFromFile(joinPathPy('molecules/13rsites.mol', __file__)).smiles())
print(indigo.loadMoleculeFromFile(joinPathPy('molecules/1e-0.mol', __file__)).smiles())
