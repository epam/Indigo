import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

indigo = Indigo()
structures = [ "CN(c1ccccc1)C=O", "CN(c1ccc(O)cc1)C=OP" ]
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

def testDecoRecursiveSmarts ():
    mols = []
    for smiles in structures:
        mol = indigo.loadMolecule(smiles)
        mol.aromatize()
        mols.append(mol)
        query = indigo.loadSmarts("[#6]!@-[N;$([N]c)]!@-C=[O$(O=C)]")
    deco = indigo.decomposeMolecules(query, mols)
    full_scaf = deco.decomposedMoleculeScaffold()
    prepareStructure(full_scaf)
    print("full scaffold: " + full_scaf.molfile())
    for item in deco.iterateDecomposedMolecules():
        print(item.decomposedMoleculeHighlighted().smiles())
        mol = item.decomposedMoleculeWithRGroups()
        prepareStructure(mol)
        print(mol.molfile())
        for rg in mol.iterateRGroups():
            print("RGROUP #%d " % (rg.index()))
            if rg.iterateRGroupFragments().hasNext():
                rg_next = rg.iterateRGroupFragments().next()
                print(rg_next.molfile())
            else:     
                print("NO FRAGMENT")
  
indigo.setOption("molfile-saving-skip-date", True)
testDecoRecursiveSmarts()
