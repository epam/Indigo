import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)
mol = indigo.loadMoleculeFromFile(joinPath('molecules/stereo_either-0020.mol'))
print(mol.molfile())
