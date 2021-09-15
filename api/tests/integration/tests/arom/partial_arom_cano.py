import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

indigo = Indigo()
for m in indigo.iterateSDFile(joinPathPy('molecules/partial_arom.sdf', __file__)):
   print("Smiles: " + m.smiles())
   # count number of aromatic bonds
   arom_bonds = len([1 for b in m.iterateBonds() if b.bondOrder() == 4])
   print("  Aromatic bonds: %d" % arom_bonds) 
   m2 = indigo.loadMolecule(m.smiles())
   print("Reloaded smiles: " + m2.smiles())
   arom_bonds2 = len([1 for b in m2.iterateBonds() if b.bondOrder() == 4])
   print("  Aromatic bonds: %d" % arom_bonds2)
   if arom_bonds != arom_bonds2:
      sys.stderr.write("Number of aromatic bonds (%d and %d) is different in %s and %s.\n" % 
         (arom_bonds, arom_bonds2, m.smiles(), m2.smiles()))
   
