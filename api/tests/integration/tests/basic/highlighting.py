import os
import sys
import errno

sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
mol = indigo.loadMolecule('C1C=NC=CN=1')
mol.aromatize()
q = indigo.loadQueryMolecule("C:*:C")
m = indigo.substructureMatcher(mol).match(q)
ht = m.highlightedTarget()

if not os.path.exists(joinPath("out")):
    try:
        os.makedirs(joinPath("out"))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

saver = indigo.createFileSaver(joinPath("out/highlighting.sdf"), "sdf")
   
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

print("*** Highlighting carbons ***")
m = indigo.loadMolecule("CC(CCl)OC(C)CCl")
for a in m.iterateAtoms():
    if a.symbol() == "C" and (a.countHydrogens() == 1 or a.countHydrogens() == 3):
        a.highlight()
        
saver.append(m)

print("*** Highlighting target ***")
mol1 = indigo.loadQueryMolecule("CN")
mol2 = indigo.loadMolecule("P(=O)(O[H])(O[H])OC([H])([H])C1=C([H])N=C(C([H])([H])[H])N=C1N([H])[H]")
mol2.foldHydrogens()
m = indigo.substructureMatcher(mol2).match(mol1)
hi = m.highlightedTarget()
print(hi.smiles())
