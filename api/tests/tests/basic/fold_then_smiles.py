import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
def foldThenSmiles(filename):
  print filename
  mol = indigo.loadMoleculeFromFile(filename)
  mol.foldHydrogens()
  print mol.smiles()
  mol.aromatize()
  print mol.canonicalSmiles()
  print
foldThenSmiles('molecules/li-h.mol')
foldThenSmiles('molecules/pc-438107.mol')
foldThenSmiles('molecules/pc-20749491.mol')
