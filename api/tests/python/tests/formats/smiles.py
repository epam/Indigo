import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

indigo = Indigo()

print("*** Wiggly bonds ***")
m = indigo.loadMolecule("CC(C)(N)O |w:1.2,1.1|")
print(m.smiles())
print(m.isChiral())
assert m.getAtom(1).stereocenterType() == Indigo.EITHER

print("*** Chrial ***")
m = indigo.loadMolecule("C[C@](N)(O)S")
print(m.smiles())
print(m.isChiral())
assert m.getAtom(1).stereocenterType() == Indigo.ABS

print("*** Not chrial ***")
m = indigo.loadMolecule("C[C@](N)(O)S |r|")
print(m.smiles())
print(m.isChiral())
assert m.getAtom(1).stereocenterType() == Indigo.AND

print("*** Relative save/load ***")
m = indigo.loadMolecule("C[C@](N)(O)S |r|")
print(m.smiles())
buf = m.serialize()
m2 = indigo.unserialize(buf)
print(m2.smiles())
assert not m.isChiral()
assert not m2.isChiral()

print("*** Either cis-trans ***")
m = indigo.loadMolecule("CC=CN |w:1.0|")
print(m.smiles())
m = indigo.loadMolecule("CC=CN")
print(m.smiles())
m = indigo.loadMolecule("C\\C=C\\N")
print(m.smiles())
m = indigo.loadMolecule("C\\C=C/N")
print(m.smiles())

print("*** Aromatic cis-trans ***")
mols_smiles = [
    "O=CC=C1N=c2ccccc2=N1 |c:11,t:4|",
    "C\\N=c1/cccc/c/1=N/C=C\\C=O",
    "O\\N=c1/c(=O)c2ccccc2c(=O)/c/1=N\\O"
]
for sm in mols_smiles:
        m = indigo.loadMolecule(sm)
        print(m.smiles())
        print(m.canonicalSmiles())

print("*** Closing with cis-trans constraint ***")
mols_smiles = [
    "CCSc1nnc2c(OC=Nc3ccccc-23)n1 |c:9|",
    "CCSc1nnc-2c(OC=Nc3ccccc-23)n1 |c:9|"
]
for sm in mols_smiles:
    print(indigo.loadMolecule(sm).smiles())

print("*** Atom lists ***")
mols_qsmiles = [
    "[#6,#7,#8]c1nc2ccccc2n1",
    "[C,N,O]c1nc2ccccc2n1",
    "[c,n,o]c1nc2ccccc2n1",
    "C1=CC=CC2=C1N=C(N2)[*;#6,#7,#8]",
    "C1=CC=CC2=C1N=C(N2)[*;C,N,O]",
    "C1=CC=CC2=C1N=C(N2)[*;c,n,o]",
]
for idx, sm in enumerate(mols_qsmiles):
    print(sm)
    q = indigo.loadQueryMolecule(sm)
    try:
        sm2 = q.smiles()
        print("  -> " + sm2)
        q2 = indigo.loadQueryMolecule(sm2)
        sm3 = q2.smiles()
        print("  -> " + sm3)
    except IndigoException as e:
        print(getIndigoExceptionText(e))

