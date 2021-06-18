import sys
sys.path.append('../../common')
from env_indigo import *
from itertools import product

indigo = Indigo()
mol_list = list(indigo.iterateSDFile(joinPath("molecules/thiazolidines_slice.sdf")))
pairs = product(mol_list, mol_list)
for (p1, p2) in pairs:
   print("%s %s" %(p1.name(), p2.name()))
   print("  tversky sim:         %0.4f" % (indigo.similarity(p1, p2, "tversky")))
   print("  tversky sim 1 0:     %0.4f" % (indigo.similarity(p1, p2, "tversky 1 0")))
   print("  tversky sim 0 1:     %0.4f" % (indigo.similarity(p1, p2, "tversky 0 1")))
   print("  tversky sim 0.7 0.3: %0.4f" % (indigo.similarity(p1, p2, "tversky 0.7 0.3")))
   print("  tanimo to:           %0.4f" % (indigo.similarity(p1, p2, None)))
   print("  euclid-sub:          %0.4f" % (indigo.similarity(p1, p2, "euclid-sub")))
