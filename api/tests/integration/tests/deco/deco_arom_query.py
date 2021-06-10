import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
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
structures = [ "OC1=CC=C2NC=CC2=C1", 
  "SC1=CC=C2NC=CC2=C1",
  "PC1=CC=C2NC=CC2=C1",
  "ON1C=CC2=CC(O)=CC=C12",
  "CN1N=CC2=CC(S)=CC=C12",
  "ON1N=CC2=CC(S)=C(F)C=C12",
  "NC1=C(S)C=C2C=NN(O)C2=C1" ]

def testQueryAromDeco ():
    mols = []
    for smiles in structures:
        mol = indigo.loadMolecule(smiles)
        mol.aromatize()
        mols.append(mol)
    query = indigo.loadQueryMolecule("N1*=CC2=CC=CC=C12")
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
            print("RGROUP # %d" % (rg.index()))
            if rg.iterateRGroupFragments().hasNext():
                print(rg.iterateRGroupFragments().next().molfile())
            else:
                print("NO FRAGMENT")
  
indigo.setOption("molfile-saving-skip-date", True)
testQueryAromDeco()
