import sys
sys.path.append('../../common')
from env_indigo import *

#
# Prepare a molecule for printing out
#
def prepareStructure(mol):
   for atom in mol.iterateAtoms():
      atom.setXYZ(0, 0, 0)
   for rg in mol.iterateRGroups():
      if rg.iterateRGroupFragments().hasNext():
         rg_next = rg.iterateRGroupFragments().next()
         for atom in rg_next.iterateAtoms():
            atom.setXYZ(0, 0, 0)

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)


def printMolfile(mol):
    smiles = mol.canonicalSmiles()
    print("Smiles: " + smiles)
    for format in ["2000", "3000", "auto"]:
        print("Format: " + format)
        indigo.setOption("molfile-saving-mode", format)
        molfile = mol.molfile()
        print(molfile)
        # Check correctness by loading molfile and saving into smiles
        sm2 = indigo.loadMolecule(molfile).canonicalSmiles()
        if smiles != sm2:
            sys.stderr.write("Error: smiles are different\n  %s\n  %s\n" % (smiles, sm2))
    indigo.setOption("molfile-saving-mode", "auto")

def printQMolfile(qmol):
    smiles = qmol.smiles()
    print("Smiles: " + smiles)
    for format in ["2000", "3000", "auto"]:
        print("Format: " + format)
        indigo.setOption("molfile-saving-mode", format)
        molfile = qmol.molfile()
        print(molfile)
    indigo.setOption("molfile-saving-mode", "auto")
    
def printRGroups(mol):
   print("separate RGroups: ")
   for rg in mol.iterateRGroups():
      print("RGROUP # %d" % (rg.index()))
      if rg.iterateRGroupFragments().hasNext():
         print(rg.iterateRGroupFragments().next().molfile())
      else:
         print("NO FRAGMENT")

def testDeco (scaf, structures):
   scaffold = indigo.loadQueryMolecule(scaf)
   deco = indigo.createDecomposer(scaffold)
   for smiles in structures:
      str = indigo.loadMolecule(smiles)
      item = deco.decomposeMolecule(str)
      
      print("highlighted structure: " + item.decomposedMoleculeHighlighted().smiles())
      print("molecule scaffold: " + item.decomposedMoleculeScaffold().smiles())
      print("molecule with rgroups: ")
      mol = item.decomposedMoleculeWithRGroups()
      prepareStructure(mol)
      printMolfile(mol)
      printRGroups(mol)
      
   full_scaf = deco.decomposedMoleculeScaffold()
   prepareStructure(full_scaf)
   print("full scaffold: ")
   printQMolfile(full_scaf)

def testDecoIterate (scaffold, structures):
   deco = indigo.createDecomposer(scaffold)
   for smiles in structures:
      str = indigo.loadMolecule(smiles)
      item = deco.decomposeMolecule(str)
      match_idx = 1
      for q_match in item.iterateDecompositions():
         print("MATCH # %d" % match_idx)
         print("highlighted structure: " + q_match.decomposedMoleculeHighlighted().smiles())
         print("molecule scaffold: " + q_match.decomposedMoleculeScaffold().smiles())
         print("molecule with rgroups: ")
         mol = q_match.decomposedMoleculeWithRGroups()
         prepareStructure(mol)
         printMolfile(mol)
         printRGroups(mol)
         deco.addDecomposition(q_match)
         match_idx += 1

   full_scaf = deco.decomposedMoleculeScaffold()
   prepareStructure(full_scaf)
   print("full scaffold: ")
   printQMolfile(full_scaf)

def testDecoIterateSmile (scaf, structures):
   scaffold = indigo.loadQueryMolecule(scaf)
   testDecoIterate (scaffold, structures)

def testDecoIterateFile (scaf, structures):
   scaffold = indigo.loadQueryMoleculeFromFile(scaf)
   testDecoIterate (scaffold, structures)

print("should decompose molecules with iteration api*************************************************************************")
testDeco("C1=CC=CC=C1", ["COCC1=C(N=CC2=C1C1=C(OC3=CC=C(Cl)C=C3)C=CC=C1N2)C(=O)OC(C)C",
                        "COCC1=CN=C(C(=O)OC(C)C)C2=C1C1=CC=C(OC3=CC=C(Cl)C=C3)C=C1N2"])

print("should have only one match for simple molecule*************************************************************************")
testDecoIterateSmile("C1CCCCC1", ["OC1CCCCC1"])

print("should have only two matches for simple molecule*************************************************************************")
testDecoIterateSmile("C1CCCCC1", ["OC1(N)CCCCC1"])

print("should add all match to full scaffold*************************************************************************")
testDecoIterateSmile("C1CCCCC1", ["NC1CCC(CC1O)C1CCC(N)C(N)C1N"])

print("should add all matches to full scaffold within decomposition*************************************************************************")
testDecoIterateSmile("C1CCCCC1", ["NC1CCC(CC1O)C1CCCCC1N"])

print("should save only one rsite*************************************************************************")
testDecoIterateSmile("C1CCCCC1", ["NC1CCCCC1", "C1CC(O)CCC1"])

print("user defined scaffold should have only two matches for simple molecule*************************************************************************")
testDecoIterateFile(joinPathPy("molecules/deco_user_def1.mol", __file__), ["NC1CCCC(O)C1"])

print("user defined scaffold should have only 4 matches for molecule*************************************************************************")
testDecoIterateFile(joinPathPy("molecules/deco_user_def1.mol", __file__), ["NC1CCCC(C1)C1CCCC(O)C1"])

print("user defined scaffold should have no matches for molecule with connected atoms*************************************************************************")
testDecoIterateFile(joinPathPy("molecules/deco_user_def1.mol", __file__), ["CC1CCC(O)CC1C"])

print("should save attachment points bond orders*************************************************************************")
indigo.setOption("deco-save-ap-bond-orders", True)
testDecoIterateSmile("C1CCCCC1", ["OC1(N)CCCCC1", "O=C1CCCCC1"])
indigo.setOption("deco-save-ap-bond-orders", False)

