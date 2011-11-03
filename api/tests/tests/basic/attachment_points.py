import os
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")
if not os.path.exists("out"):
   os.makedirs("out")
outfile = "out/replaced_r_fragments.sdf"
replaced_r_fragments = indigo.createFileSaver(outfile, "sdf")
m1 = indigo.loadMolecule("CC1=CC2CC3CCCCC3CC2C=C1O")
m2 = indigo.loadMolecule("CC1=CC(CO)C(CF)C(C)=C1O")
m3 = indigo.loadMolecule("CC1=CC2COCC2C(C)=C1O")
m4 = indigo.loadMolecule("CC1=C2OCCC2=C(O)C2COCC12")
arr = [m1, m2, m3, m4]
for m in arr:
   m.layout()
scaf = indigo.extractCommonScaffold(arr, "")
print "scaffold: " + scaf.smiles()
deco = indigo.decomposeMolecules(scaf, arr)
full_scaf = deco.decomposedMoleculeScaffold()
print "full scaffold: " + full_scaf.smiles()
def printAttachmentPoints(mol, offset):
   count = mol.countAttachmentPoints()
   print("%s Number of attachment points: %s" % (offset, count))
   for order in range(1, count + 1):
      for a in mol.iterateAttachmentPoints(order):
         print("%s   Index: %d. Order %d" % (offset, a.index(), order))
def processRGroup(mol, offset):
   printAttachmentPoints(mol, offset)
   mol2 = indigo.loadMolecule(mol.molfile())
   printAttachmentPoints(mol2, offset)
   mol2.clearAttachmentPoints()
   printAttachmentPoints(mol2, offset)
   mol3 = indigo.loadMolecule(mol.molfile())
   replaceAttachmentPointsWithLayout(mol3, offset)
   
def replaceAttachmentPointsWithLayout(mol, offset):
   count = mol.countAttachmentPoints()
   print("%s Number of attachment points: %s" % (offset, count))
   atoms = []
   for order in range(1, count + 1):
      for a in mol.iterateAttachmentPoints(order):
         atoms.append(a)
   mol.clearAttachmentPoints()
   added_atoms = []
   for a in atoms:
      a2 = mol.addAtom("I")
      a.addBond(a2, 1)
      added_atoms.append(a2.index())
      
   submol = mol.getSubmolecule(added_atoms)
   submol.layout()
   replaced_r_fragments.append(mol)
   
   print(mol.canonicalSmiles())
   print(mol.molfile())
for item in deco.iterateDecomposedMolecules():
   print("Molecule: %s" % (item.decomposedMoleculeHighlighted().smiles()))
   mol = item.decomposedMoleculeWithRGroups()
   print "  decomposed molecule: " + mol.canonicalSmiles()
   print "  mapped scaffold: " + item.decomposedMoleculeScaffold().canonicalSmiles()
   for rg in mol.iterateRGroups():
      print "    RGROUP #" + str(rg.index())
      if rg.iterateRGroupFragments().hasNext():
         frag = rg.iterateRGroupFragments().next()
         print "      fragment #" + str(frag.index()) + ":", frag.canonicalSmiles()
         processRGroup(frag, "        ")
      else:
         print "      NO FRAGMENT"
