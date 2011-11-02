import os;
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
print indigo.loadMoleculeFromFile('molecules/13rsites.mol').smiles()
print indigo.loadMoleculeFromFile('molecules/1e-0.mol').smiles()
