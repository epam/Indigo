import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
structures = [ "CN(c1ccccc1)C=O", "CN(c1ccc(O)cc1)C=OP" ]
def testDecoRecursiveSmarts ():
  mols = []
  for smiles in structures:
    mol = indigo.loadMolecule(smiles)
    mol.aromatize()
    mols.append(mol)
  query = indigo.loadSmarts("[#6]!@-[N;$([N]c)]!@-C=[O$(O=C)]")
  deco = indigo.decomposeMolecules(query, mols)
  full_scaf = deco.decomposedMoleculeScaffold()
  print "full scaffold: " + full_scaf.molfile()
  for item in deco.iterateDecomposedMolecules():
    print item.decomposedMoleculeHighlighted().smiles()
    mol = item.decomposedMoleculeWithRGroups()
    print mol.molfile()
    for rg in mol.iterateRGroups():
      print "RGROUP #", rg.index()
      if rg.iterateRGroupFragments().hasNext():
        print rg.iterateRGroupFragments().next().molfile()
      else:
        print "NO FRAGMENT"
  
indigo.setOption("molfile-saving-skip-date", True)
testDecoRecursiveSmarts()
