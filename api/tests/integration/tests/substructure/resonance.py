import os
import sys

sys.path.append("../../common")
from env_indigo import *

indigo = Indigo()


def testSingleResonanceMatch(mol1, mol2, expected, hl):
    mol1 = indigo.loadQueryMolecule(mol1)
    mol2 = indigo.loadMolecule(mol2)
    match = indigo.substructureMatcher(mol2, "RES").match(mol1)
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


testSingleResonanceMatch("C=N", "C=C[N-H]", True, True)
testSingleResonanceMatch("C=C", "[C-H2]C=N", True, False)
testSingleResonanceMatch(
    "O=[*-]([O-])([O-])", "B([O-])([O-])[O-]", True, False
)
testSingleResonanceMatch(
    "[*]~[B-]([*-])([*-])", "B([O-])([O-])[O-]", True, False
)
testSingleResonanceMatch(
    "N-C(~[O-])~C~C(~[O-])-C", "CC(=O)[CH-]C(N)=O", False, False
)
testSingleResonanceMatch("[C+]-[O-]", "C=O", False, False)
testSingleResonanceMatch("CC(O)=CC(N)=O", "CC(=O)[CH-]C(N)=O", True, False)
testSingleResonanceMatch(
    "[NH-]C=CC=[*][*]=[*][*]=[*]C=CC=O", "[O-]C=CC=CC=CC=CC=CC=N", True, False
)
testSingleResonanceMatch("C[B-](=O)NN=C", "OB1NN=CC=C1", False, False)
