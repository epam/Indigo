import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
print(indigo.loadMoleculeFromFile(joinPath('molecules/13rsites.mol')).smiles())
print(indigo.loadMoleculeFromFile(joinPath('molecules/1e-0.mol')).smiles())
