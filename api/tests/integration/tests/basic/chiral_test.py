import os
import sys
import errno

sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
set = [m for m in indigo.iterateSDFile(joinPathPy('molecules/chiral_test.sdf', __file__))]
def calculateCanonicalSmiles (set):
   sm_tuples = []
   for m in set:
      tuple = (m.canonicalSmiles(), m.isChiral())
      print("%s. Chiral: %s" % tuple)
      sm_tuples.append(tuple)
   return sm_tuples
print("Chiral flag and canonical smiles:")
calculateCanonicalSmiles(set)
print("Change stereocenters")
   
for m in set:
   for a in m.iterateStereocenters():
      a.changeStereocenterType(Indigo.ABS)
      
res1 = calculateCanonicalSmiles(set)

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

outfile = joinPathPy("out/chiral_test_out.sdf", __file__)
saver = indigo.createFileSaver(outfile, "sdf")
for m in set:
   saver.append(m)
   
saver.close()
print("Reload and check")
set2 = [m for m in indigo.iterateSDFile(outfile)]
res2 = calculateCanonicalSmiles(set2)
for t1, t2 in zip(res1, res2):
   if t1 != t2:
      print("Error: %s != %s" % (t1, t2))
