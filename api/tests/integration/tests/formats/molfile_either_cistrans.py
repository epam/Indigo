import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)
mol = indigo.loadMoleculeFromFile(joinPathPy('molecules/stereo_either-0020.mol', __file__))
print(mol.molfile())
