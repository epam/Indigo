import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()


def testSingleTauMatch(mol1, mol2, flags, expected, hl):
    match = indigo.substructureMatcher(mol2, flags).match(mol1)
    if match:
        sys.stdout.write("matched")
    else:
        sys.stdout.write("unmatched")
    if (match is None) == expected:
        sys.stdout.write("(unexpected)")
    print("")
    if match and hl:
        print(match.highlightedTarget().smiles())
        for atom in mol1.iterateAtoms():
            mapped = match.mapAtom(atom)
            if mapped is None:
                mapped = "?"
            else:
                mapped = mapped.index()
            print("{0} -> {1}".format(atom.index(), mapped))


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
    testSingleTauMatch(mol1, mol2, "TAU INCHI", False, False)
    mol1 = indigo.loadQueryMolecule("C=C1CCC(=C)CC1")
    mol2 = indigo.loadMolecule("CC(C)=C1CCC(C)=CC1")
    testSingleTauMatch(mol1, mol2, "TAU", True, True)
    testSingleTauMatch(mol1, mol2, "TAU R*", False, False)
    testSingleTauMatch(mol1, mol2, "TAU INCHI", False, False)
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
    testSingleTauMatch(mol1, mol2, "TAU INCHI", False, False)
    mol1 = indigo.loadQueryMolecule("OCCCN")
    mol2 = indigo.loadMolecule("O=C1N=CNC2=C1C=NN2")
    testSingleTauMatch(mol1, mol2, "TAU", False, False)
    testSingleTauMatch(mol1, mol2, "TAU INCHI", True, True)


testTauSubFlags()

print("*** Specific cases ***")
indigo.clearTautomerRules()
indigo.setTautomerRule(1, "N,O,P,S,As,Se,Sb,Te", "N,O,P,S,As,Se,Sb,Te")
indigo.setTautomerRule(2, "0C", "N,O,P,S")
indigo.setTautomerRule(3, "1C", "N,O")

mol1 = indigo.loadQueryMolecule("CCC")
mol2 = indigo.loadMolecule(
    "P(=O)(O[H])(O[H])OC([H])([H])C1=C([H])N=C(C([H])([H])[H])N=C1N([H])[H]"
)
mol2.foldHydrogens()
mol2.aromatize()
testSingleTauMatch(mol1, mol2, "TAU", True, True)

mol3 = indigo.unserialize(mol2.serialize())
testSingleTauMatch(mol1, mol3, "TAU", True, True)
