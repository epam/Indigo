import os
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
mol = indigo.loadMolecule('C1C=NC=CN=1')
mol.aromatize()
q = indigo.loadQueryMolecule("C:*:C")
m = indigo.substructureMatcher(mol).match(q)
ht = m.highlightedTarget()
if not os.path.exists("out"):
   os.makedirs("out")
saver = indigo.createFileSaver("out/highlighting.sdf", "sdf")
   
def printHighlighting (ht):
   print("Highlighting:")
   hatoms = [a.index() for a in ht.iterateAtoms() if a.isHighlighted()]
   hbonds = [b.index() for b in ht.iterateBonds() if b.isHighlighted()]
   print("  Atoms: %s" % hatoms)
   print("  Bonds: %s" % hbonds)
   saver.append(ht)
printHighlighting(ht)
for a in ht.iterateAtoms():
   if a.isHighlighted():
      a.unhighlight()
      
printHighlighting(ht)
for b in ht.iterateBonds():
   if b.isHighlighted():
      b.unhighlight()
printHighlighting(ht)
ht = m.highlightedTarget()
printHighlighting(ht)
ht.unhighlight()
printHighlighting(ht)
for a in ht.iterateAtoms():
   a.highlight()
printHighlighting(ht)
for b in ht.iterateBonds():
   b.highlight()
printHighlighting(ht)
