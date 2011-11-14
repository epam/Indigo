import sys
sys.path.append('../../common')
from env_indigo import *

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
    query = indigo.loadQueryMolecule("N1[*]=CC2=CC=CC=C12")
    deco = indigo.decomposeMolecules(query, mols)
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
  
indigo.setOption("molfile-saving-skip-date", True)
testQueryAromDeco()
