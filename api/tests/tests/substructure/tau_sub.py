import random;
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
def testSingleTauMatch (mol1, mol2, flags, expected, hl):
  match = indigo.substructureMatcher(mol2, flags).match(mol1)
  if match:
    print "matched",
  else:
    print "unmatched",
  if (match is None) == expected:
    print "(unexpected)",
  print
  if match and hl:
    print match.highlightedTarget().smiles()
    for atom in mol1.iterateAtoms():
      mapped = match.mapAtom(atom)
      if mapped is None:
        mapped = '?'
      else:
        mapped = mapped.index()
      print atom.index(), '->', mapped
def testTauSubFlags():
  indigo.clearTautomerRules()
  indigo.setTautomerRule(1, "N,O,P,S,As,Se,Sb,Te", "N,O,P,S,As,Se,Sb,Te")
  indigo.setTautomerRule(2, "0C", "N,O,P,S")
  indigo.setTautomerRule(3, "1C", "N,O")
  mol1 = indigo.loadQueryMolecule("OC1=CC=CNC1")
  mol2 = indigo.loadMolecule("O=C1CNCC2=CC=CC=C12")
  testSingleTauMatch(mol1, mol2, "TAU", True, True)
  testSingleTauMatch(mol1, mol2, "TAU R2", True, False)
  testSingleTauMatch(mol1, mol2, "TAU R1 R3", False, False)
  mol1 = indigo.loadQueryMolecule("C=C1CCC(=C)CC1")
  mol2 = indigo.loadMolecule("CC(C)=C1CCC(C)=CC1")
  testSingleTauMatch(mol1, mol2, "TAU", True, True)
  testSingleTauMatch(mol1, mol2, "TAU R*", False, False)
  mol1 = indigo.loadQueryMolecule("NC1=NC2=NC=CN=C2C=N1")
  mol2 = indigo.loadMolecule("NC1=NC2=C(N=C(C=N2)C(O)=O)C(=O)N1")
  testSingleTauMatch(mol1, mol2, "TAU R1", True, True)
  testSingleTauMatch(mol1, mol2, "TAU R2 R3", False, False)
  mol2 = indigo.loadMolecule("NC1=NC2=C(N=CC(=O)N2)C(=O)N1")
  testSingleTauMatch(mol1, mol2, "TAU R1", True, True)
  testSingleTauMatch(mol1, mol2, "TAU R2 R3", False, False)
  mol1 = indigo.loadQueryMolecule("CC1(C)NC(=O)C2=CC=CC=C2N1")
  mol2 = indigo.loadMolecule("CC(=C)NC1=CC=CC=C1C(N)=O")
  mol2.aromatize()
  testSingleTauMatch(mol1, mol2, "TAU R-C", True, False)
  testSingleTauMatch(mol1, mol2, "TAU R-C R2", True, False)
  testSingleTauMatch(mol1, mol2, "TAU", False, False)
  testSingleTauMatch(mol1, mol2, "TAU R2 R3", False, False)
  
testTauSubFlags()
