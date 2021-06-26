import sys

sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()


def testSingleTauMatch(mol1, mol2, flags, expected, hl):
    match = indigo.substructureMatcher(mol2, flags).match(mol1)
    if match:
        sys.stdout.write("matched")
    else:
        sys.stdout.write("unmatched")
    if (match is None) == expected:
        sys.stdout.write("(unexpected)")
    print('')
    if match and hl:
        print(match.highlightedTarget().smiles())
        for atom in mol1.iterateAtoms():
            mapped = match.mapAtom(atom)
            if mapped is None:
                mapped = '?'
            else:
                mapped = mapped.index()
            print('{0} -> {1}'.format(atom.index(), mapped))


def testTauSubFlags():
    indigo.clearTautomerRules()
    indigo.setTautomerRule(1, "N,O,P,S,As,Se,Sb,Te", "N,O,P,S,As,Se,Sb,Te")
    indigo.setTautomerRule(2, "0C", "N,O,P,S")
    indigo.setTautomerRule(3, "1C", "N,O")
    mol1 = indigo.loadQueryMolecule("CC1(C)NC(=O)C2=CC=CC=C2N1")
    mol2 = indigo.loadMolecule("CC(=C)NC1=CC=CC=C1C(N)=O")
    mol2.aromatize()
    testSingleTauMatch(mol1, mol2, "TAU R-C", True, False)
    testSingleTauMatch(mol1, mol2, "TAU R-C R2", True, False)

testTauSubFlags()
