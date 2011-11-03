import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
m1 = indigo.loadMolecule("OC[C@H]1O[C@H](O[C@@H]2[C@@H](CO)O[C@@H](OCC3CCCCC3)[C@H](O)[C@H]2O)[C@H](O)[C@@H](O)[C@@H]1O")
m2 = indigo.loadMolecule("OC[C@H]1O[C@H](O[C@@H]2[C@@H](O)C(O)O[C@H](CO)[C@H]2O)[C@H](O)[C@@H](O)[C@@H]1O")
m3 = indigo.loadMolecule("[Cl-].COC1=CC(=CC(OC)=C1O)C1=C(O[C@H]2O[C@H](CO)[C@@H](O)[C@H](O)[C@H]2O)C=C2C(O[C@H]3O[C@H](CO)[C@@H](O)[C@H](O)[C@H]3O)=CC(O)=CC2=[O+]1")
m4 = indigo.loadMolecule("COC(=O)CCSCCO[C@@H]1O[C@H](CO)[C@@H](O[C@H]2O[C@H](CO)[C@@H](O[C@H]3O[C@H](CO[C@H]4O[C@H](CO)[C@@H](O)[C@H](O)[C@H]4O)[C@@H](O)[C@H](O)[C@H]3O)[C@H](O)[C@H]2O)[C@H](O)[C@H]1O")
m1.layout()
m2.layout()
m3.layout()
m4.layout()
array = [m1, m2, m3, m4]
scaffold = indigo.loadQueryMolecule("OCC1OC(O)C(O)C(O)C1O")
print("Should correctly save highlighted structues")
decomp_iter = indigo.decomposeMolecules(scaffold, array)
cnt = 0
for decomp in decomp_iter.iterateDecomposedMolecules():
   print "%d:" % cnt
   cnt += 1
   
   high_mol = decomp.decomposedMoleculeHighlighted()
   indigo.setOption("molfile-saving-mode", "2000")
   mol1 = indigo.loadMolecule(high_mol.molfile())
   indigo.setOption("molfile-saving-mode", "3000")
   mol2 = indigo.loadMolecule(high_mol.molfile())
   print("  sm1: " + mol1.smiles())
   print("  sm2: " + mol2.smiles())
   mol1.unhighlight()
   mol2.unhighlight()
   sm1 = mol1.canonicalSmiles()
   sm2 = mol2.canonicalSmiles()
   print("  can sm1: " + sm1)
   print("  can sm2: " + sm2)
   if sm1 != sm2:
      sys.stderr.write("%s != %s\n" % (sm1, sm2))
