import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

indigo = Indigo()
def testExactMatchMolsUnmappedH ():
    mol1 = indigo.loadMolecule("C([H])1=CC=CC=C1")
    mol2 = indigo.loadMolecule("C1=CC=C([H])C=C1")
    match = indigo.exactMatch(mol1, mol2)
    for atom in mol1.iterateAtoms():
        mapped = match.mapAtom(atom)        
        if not mapped:
            index = '?'
        else:
            index = mapped.index()
        sys.stdout.write(str(index) + ' ')
    sys.stdout.write('\n')
    
def testSingleMatch (mol1, mol2, flags, expected):
    match = indigo.exactMatch(mol1, mol2, flags)
    if match:
        sys.stdout.write("matched")
    else:
        sys.stdout.write("unmatched")
    if (match is None) == expected:
        sys.stdout.write(" (unexpected)")
    sys.stdout.write('\n')

def testExactMatchMolsFlags ():
    mol1 = indigo.loadMolecule("C1CCCCC1")
    mol2 = indigo.loadMolecule("c1ccccc1")
    testSingleMatch(mol1, mol2, None, False)
    testSingleMatch(mol1, mol2, "FRA MAS STE", True)
    testSingleMatch(mol1, mol2, "ALL -ELE", True)
    mol1 = indigo.loadMolecule("[N]")
    mol2 = indigo.loadMolecule("[14N]")
    testSingleMatch(mol1, mol2, "MAS STE", False)
    testSingleMatch(mol1, mol2, "STE", True)
    mol1 = indigo.loadMolecule("[C@](C)(O)N")
    mol2 = indigo.loadMolecule("[C@@](C)(O)N")
    testSingleMatch(mol1, mol2, "STE", False)
    testSingleMatch(mol1, mol2, "ALL -STE", True)
    mol1 = indigo.loadMolecule("[C@](C)(O)N")
    mol2 = indigo.loadMolecule("[C@@](C)(O)N.Cl")
    testSingleMatch(mol1, mol2, "STE", False)
    testSingleMatch(mol1, mol2, "FRA", False)
    testSingleMatch(mol1, mol2, "ALL -STE", False)
    testSingleMatch(mol1, mol2, "ALL -STE -FRA", True) 
    testSingleMatch(mol1, mol2, "MAS ELE", True)
    
def testTautomerMatchUnmappedH ():
    mol1 = indigo.loadMolecule("C([H])1=CC=CC=C1")
    mol2 = indigo.loadMolecule("C1=CC=C([H])C=C1")
    match = indigo.exactMatch(mol1, mol2, "TAU")
    for atom in mol1.iterateAtoms():
        mapped = match.mapAtom(atom)
        if not mapped:
            index = '?'
        else:
            index = mapped.index()
        sys.stdout.write(str(index) + ' ')
    sys.stdout.write('\n')
  
def testTautomerMatchFlags ():
    indigo.clearTautomerRules()
    indigo.setTautomerRule(1, "N,O,P,S,As,Se,Sb,Te", "N,O,P,S,As,Se,Sb,Te")
    indigo.setTautomerRule(2, "0C", "N,O,P,S")
    indigo.setTautomerRule(3, "1C", "N,O")
    
    mol1 = indigo.loadMolecule("CC1(C)NC(=O)C2=CC=CC=C2N1")
    mol2 = indigo.loadMolecule("CC(=C)NC1=CC=CC=C1C(N)=O")
    mol2.aromatize()
    testSingleMatch(mol1, mol2, "TAU R-C", True)
    testSingleMatch(mol1, mol2, "TAU R-C R2", True)
    testSingleMatch(mol1, mol2, "TAU", False)
    testSingleMatch(mol1, mol2, "TAU R2 R3", False)
    mol1 = indigo.loadMolecule("OC1=C2C=CC=CC2=CCC1")
    mol2 = indigo.loadMolecule("O=C1CCCC2=C1C=CC=C2")
    mol2.aromatize()
    testSingleMatch(mol1, mol2, "TAU", True)
    testSingleMatch(mol1, mol2, "TAU R2", True)
    testSingleMatch(mol1, mol2, "TAU R1 R3", False)
    mol1 = indigo.loadMolecule("OC1=C(N=NC2=CC=CC=C2)C2=CC=CC=C2C=C1")
    mol2 = indigo.loadMolecule("O=C1C=CC2=CC=CC=C2\C1=N/NC1=CC=CC=C1")
    mol2.aromatize()
    testSingleMatch(mol1, mol2, "TAU", True)
    testSingleMatch(mol1, mol2, "TAU R1", True)
    testSingleMatch(mol1, mol2, "TAU R2 R3", False)
    mol1 = indigo.loadMolecule("NC1=NC2=C(N=CC(O)=N2)C(O)=N1")
    mol2 = indigo.loadMolecule("NC1=NC2=C(N=CC(=O)N2)C(=O)N1")
    testSingleMatch(mol1, mol2, "TAU", True)
    testSingleMatch(mol1, mol2, "TAU R1", True)
    testSingleMatch(mol1, mol2, "TAU R2 R3", False)
    mol1 = indigo.loadMolecule("CC1=CCC(=C)CC1")
    mol2 = indigo.loadMolecule("C=C1CCC(=C)CC1")
    testSingleMatch(mol1, mol2, "TAU", True)
    testSingleMatch(mol1, mol2, "TAU R*", False)
    
def testExactReactionMatchUnmappedH ():
    rxn1 = indigo.loadReaction("CCO[H]>CCC>OCC")
    rxn2 = indigo.loadReaction("OCC>CC>CCO")
    match = indigo.exactMatch(rxn1, rxn2)
    for mol in rxn1.iterateMolecules():
        print('mol  {0}'.format(mol.index()))
        for atom in mol.iterateAtoms():
            mapped = match.mapAtom(atom)
            if not mapped:
                mapped = '?'
            else:
                mapped = mapped.index()
            print('atom  {0} -> {1}'.format(atom.index(), mapped))
    print(match.highlightedTarget().smiles())
    
def testReactionSingleMatch (rxn1, rxn2, expected):
    rxn1 = indigo.loadReaction(rxn1)
    rxn2 = indigo.loadReaction(rxn2)
    rxn1.aromatize()
    rxn2.aromatize()
    match = indigo.exactMatch(rxn1, rxn2)
    if match:
        sys.stdout.write("matched")
    else:
        sys.stdout.write("unmatched")
    if (match is None) == expected:
        sys.stdout.write(" (unexpected)")
    sys.stdout.write('\n')
 
def testExactReactionFlags ():
    rxn1 = indigo.loadReaction("C1CCCCC1>>N")
    rxn2 = indigo.loadReaction("c1ccccc1>>[13NH3]")
    testSingleMatch(rxn1, rxn2, None, False)
    testSingleMatch(rxn1, rxn2, "STE AAM", True)
    testSingleMatch(rxn1, rxn2, "ALL -ELE -MAS", True)
    rxn1 = indigo.loadReaction("[C@](C)([OH:1])N>>C[OH:1]")
    rxn2 = indigo.loadReaction("[C@@](C)(O)N>>CO")
    testSingleMatch(rxn1, rxn2, "STE", False)
    testSingleMatch(rxn1, rxn2, "AAM", False)
    testSingleMatch(rxn1, rxn2, "ALL -STE -AAM", True)
    testSingleMatch(rxn1, rxn2, "MAS ELE RCT", True)
    testReactionSingleMatch("N.C1C=CC=CC=1>>C(=O)OP", "C1=CC=CC=C1.N>>C(=O)OP", True)
    testReactionSingleMatch("N.C1C=CC=CC=1>>[14C](=O)OP", "C1=CC=CC=C1.N>>C(=O)OP", False)
    testReactionSingleMatch("c1ccccc1>>C(=O)OP", "C1=CC=CC=C1>>C(=O)OP", True)
    testReactionSingleMatch("O.N.C1C=CC=CC=1>>C(=O)OP", "C1=CC=CC=C1.N>>C(=O)OP.C", False)
  
  
testExactMatchMolsUnmappedH()
testExactMatchMolsFlags()
testTautomerMatchUnmappedH()
testTautomerMatchFlags()
testExactReactionMatchUnmappedH()
testExactReactionFlags()
