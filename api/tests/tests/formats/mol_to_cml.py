
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
mol = indigo.loadMoleculeFromFile('molecules/disconnected-0106.mol')
print mol.cml()
