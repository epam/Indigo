import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
for mol, num in zip(indigo.iterateSDFile(joinPath("molecules/valence_test1.sdf")), range(100000)):
   for atom in mol.iterateAtoms():
      try:
         atom.valence()
      except IndigoException, e:
         print("molecule #%d, atom #%d: %s" % (num, atom.index(), getIndigoExceptionText(e)))
