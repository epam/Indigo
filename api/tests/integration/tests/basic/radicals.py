import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")

print("****** Basic ********")
m = indigo.loadMolecule("C1C=CC=C1")
print(m.smiles())
a = m.getAtom(0)
print(a.radicalElectrons())
print(a.radical())

rad = [Indigo.SINGLET, Indigo.DOUBLET, Indigo.TRIPLET]

for r in rad:
    a.setRadical(r)
    print(a.radicalElectrons())
    print(a.radical() == r)
    print(m.smiles())


print("****** SDF ********")
radicalMap = {
    0: "None",
    Indigo.SINGLET: "Singlet",
    Indigo.DOUBLET: "Doublet",
    Indigo.TRIPLET: "Triplet",
}


def testRadical(m):
    print(m.smiles())
    for a in m.iterateAtoms():
        print(
            " %d: %s with %d electrons"
            % (a.index(), radicalMap[a.radical()], a.radicalElectrons())
        )


for m in indigo.iterateSDFile(joinPathPy("molecules/radicals.sdf", __file__)):
    testRadical(m)
    m2 = indigo.loadMolecule(m.molfile())
    testRadical(m2)
    m3 = indigo.loadMolecule(m.smiles())
    testRadical(m3)
