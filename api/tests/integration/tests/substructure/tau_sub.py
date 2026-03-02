import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import Indigo  # noqa

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
    # Test INNER
    mol1 = indigo.loadQueryMolecule("OC1=CC=CNC1")
    mol2 = indigo.loadMolecule("O=C1CNCC2=CC=CC=C12")
    testSingleTauMatch(mol1, mol2, "TAU INNER", False, False)
    mol1 = indigo.loadQueryMolecule("C=C1CCC(=C)CC1")
    mol2 = indigo.loadMolecule("CC(C)=C1CCC(C)=CC1")
    testSingleTauMatch(mol1, mol2, "TAU INNER", True, True)


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

# test performance
indigo.clearTautomerRules()
mol1 = indigo.loadQueryMolecule("CC(O)=CCC")
mol2 = indigo.loadMolecule(
    r"O(N1C=NC2C(N)=NC=NC1=2)C1CC(OP(=O)(O)OCC2C(OP(=O)(O)OCC3C(OP"
    r"(=O)(O)OCC4C(OP(=O)(O)OCC5C(OP(=O)(O)OCC6C(OP(=O)(O)OCC7C(OP"
    r"(=O)(O)OCC8C(OP(=O)(O)OCC9C(OP(=O)(O)OCC%10C(OP(=O)(O)OCC%11"
    r"C(OP(=O)(O)OCC%12C(OP(=O)(O)OCC%13C(OP(=O)(O)OCC%14C(OP(=O)("
    r"O)OCC%15C(OP(=O)(O)OCC%16C(OP(=O)(O)OCC%17C(OP(=O)(O)OCC%18C"
    r"(OP(=O)(O)OCC%19C(OP(=O)(O)OCC%20C(OP(=O)(O)OCC%21C(O)CC(ON%"
    r"22C=NC%23C(N)=NC=NC%22=%23)O%21)CC(ON%21C=CC(=O)NC%21=O)O%20"
    r")CC(ON%20C=NC%21C(=O)NC(N)=NC%20=%21)O%19)CC(ON%19C=CC(N)=NC"
    r"%19=O)O%18)CC(ON%18C=NC%19C(N)=NC=NC%18=%19)O%17)CC(ON%17C=C"
    r"C(=O)NC%17=O)O%16)CC(ON%16C=NC%17C(=O)NC(N)=NC%16=%17)O%15)C"
    r"C(ON%15C=CC(N)=NC%15=O)O%14)CC(ON%14C=NC%15C(N)=NC=NC%14=%15"
    r")O%13)CC(ON%13C=CC(=O)NC%13=O)O%12)CC(ON%12C=NC%13C(=O)NC(N)"
    r"=NC%12=%13)O%11)CC(ON%11C=CC(N)=NC%11=O)O%10)CC(ON%10C=NC%11"
    r"C(N)=NC=NC%10=%11)O9)CC(ON9C=CC(=O)NC9=O)O8)CC(ON8C=NC9C(=O)"
    r"NC(N)=NC8=9)O7)CC(ON7C=CC(N)=NC7=O)O6)CC(ON6C=NC7C(N)=NC=NC6"
    r"=7)O5)CC(ON5C=CC(=O)NC5=O)O4)CC(ON4C=NC5C(=O)NC(N)=NC4=5)O3)"
    r"CC(ON3C=CC(N)=NC3=O)O2)C(COP(O)(=O)O)O1"
)
testSingleTauMatch(mol1, mol2, "TAU INNER", False, False)
