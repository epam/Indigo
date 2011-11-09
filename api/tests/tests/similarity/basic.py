import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", "1")

def checkOne(mol, metric):   
   v = indigo.similarity(mol, mol, metric)
   print("  %s: %s" % (metric, v))
   if v != 1.0:
      sys.stderr.write("    mol %s, metric %s, sim = %s but must be 1\n" % (mol.name(), metric, v))

for m in indigo.iterateSDFile(joinPath("molecules/thiazolidines_slice.sdf")):
   print("%s: %s" % (m.name(), m.canonicalSmiles()))
   checkOne(m, "tversky")
   checkOne(m, "tversky 1 0")
   checkOne(m, "tversky 0 1")
   checkOne(m, "tversky 0.7 0.3")
   checkOne(m, None)
   checkOne(m, "euclid-sub")
