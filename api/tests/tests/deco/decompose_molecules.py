import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()


def testHighlightDeco(structures):
   mols = []
   for smiles in structures:
      mol = indigo.loadMolecule(smiles)
      mol.layout()
      mols.append(mol)
   
   scaffold = indigo.loadQueryMolecule("OCC1OC(O)C(O)C(O)C1O")
   decomp_iter = indigo.decomposeMolecules(scaffold, mols)
   cnt = 0
   for decomp in decomp_iter.iterateDecomposedMolecules():
      print("%d:" % cnt)
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



def testDeco (structures):
   mols = []
   for smiles in structures:
      mol = indigo.loadMolecule(smiles)
      mols.append(mol)

   scaffold = indigo.extractCommonScaffold(mols, "EXACT")
   
   deco = indigo.decomposeMolecules(scaffold, mols)
   
   full_scaf = deco.decomposedMoleculeScaffold()

   print("full scaffold: " + full_scaf.molfile())

   for item in deco.iterateDecomposedMolecules():
      print(item.decomposedMoleculeHighlighted().smiles())
      mol = item.decomposedMoleculeWithRGroups()
      print(mol.molfile())
      for rg in mol.iterateRGroups():
         print("RGROUP # {0}".format(rg.index()))
         if rg.iterateRGroupFragments().hasNext():
            print(rg.iterateRGroupFragments().next().molfile())
         else:
            print("NO FRAGMENT")


test_structures = ["OC[C@H]1O[C@H](O[C@@H]2[C@@H](CO)O[C@@H](OCC3CCCCC3)[C@H](O)[C@H]2O)[C@H](O)[C@@H](O)[C@@H]1O",
"OC[C@H]1O[C@H](O[C@@H]2[C@@H](O)C(O)O[C@H](CO)[C@H]2O)[C@H](O)[C@@H](O)[C@@H]1O",
"[Cl-].COC1=CC(=CC(OC)=C1O)C1=C(O[C@H]2O[C@H](CO)[C@@H](O)[C@H](O)[C@H]2O)C=C2C(O[C@H]3O[C@H](CO)[C@@H](O)[C@H](O)[C@H]3O)=CC(O)=CC2=[O+]1",
"COC(=O)CCSCCO[C@@H]1O[C@H](CO)[C@@H](O[C@H]2O[C@H](CO)[C@@H](O[C@H]3O[C@H](CO[C@H]4O[C@H](CO)[C@@H](O)[C@H](O)[C@H]4O)[C@@H](O)[C@H](O)[C@H]3O)[C@H](O)[C@H]2O)[C@H](O)[C@H]1O"
]
print("Should correctly save highlighted structues")
testHighlightDeco(test_structures)

print("should correctly find all rgroups small")
testDeco(["CCC", "CCC(C)C"])

print("should correctly find two att order rgroups")
testDeco(["C1CN2C3CCC3N2C1", "ON1CCCN1"])

print("should correctly find several att order rgroups")
testDeco(["ClP1CCCN1", "C1CN2C3CCC3P2C1", "OP1(O)CCCN1"])

print("should correctly find match within simple and dif rgroup")
testDeco(["ClP1CCCN1", "C1CNP2(C1)C1CCC21", "OP1(O)CCCN1"])
