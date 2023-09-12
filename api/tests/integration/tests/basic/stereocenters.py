import errno
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")

m = indigo.createMolecule()
a0 = m.addAtom("C")
a1 = m.addAtom("N")
a2 = m.addAtom("O")
a3 = m.addAtom("P")
a0.addBond(a1, 1)
a0.addBond(a2, 1)
a0.addBond(a3, 1)
print(m.smiles())
a0.addStereocenter(indigo.ABS, a1.index(), a2.index(), a3.index())
m.layout()
print(m.smiles())
m.clearStereocenters()
a4 = m.addAtom("H")
a0.addBond(a4, 1)
a0.addStereocenter(indigo.ABS, a1.index(), a2.index(), a3.index(), a4.index())
print(
    "  "
    + ",".join(
        [str(i) for i in [a1.index(), a2.index(), a3.index(), a4.index()]]
    )
)
print("  " + ",".join([str(i) for i in a0.stereocenterPyramid()]))

print(m.smiles())

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

m.layout()
m.saveMolfile(joinPathPy("out/stereocenters_out12.mol", __file__))
m.clearStereocenters()
a0.addStereocenter(indigo.ABS, a2.index(), a1.index(), a3.index(), a4.index())
print(m.smiles())
m.layout()
m.saveMolfile(joinPathPy("out/stereocenters_out21.mol", __file__))

print("****** Stereocenters pyramid ********")
indigo.setOption("treat-x-as-pseudoatom", "1")
indigo.setOption("ignore-stereochemistry-errors", "true")
for idx, m in enumerate(
    indigo.iterateSDFile(
        joinPathPy("../../../../../data/molecules/basic/sugars.sdf", __file__)
    )
):
    print("%d: %s" % (idx, m.smiles()))
    for s in m.iterateStereocenters():
        print("  " + ",".join([str(i) for i in s.stereocenterPyramid()]))

print("****** Stereocenters groups ********")
m = indigo.loadMolecule("C[C@H]1[C@H](C)[C@@H](C)[C@H](C)[C@H](C)[C@@H]1C")
for s in m.iterateStereocenters():
    s.changeStereocenterType(Indigo.OR)

print("Initial:")
for s in m.iterateStereocenters():
    print("  %d: %d" % (s.index(), s.stereocenterGroup()))
print(m.canonicalSmiles())

print("After changes:")
for i, s in enumerate(m.iterateStereocenters()):
    s.setStereocenterGroup(i)
    print("  %d: %d" % (s.index(), s.stereocenterGroup()))
print(m.canonicalSmiles())

print("Set all to one group:")
for i, s in enumerate(m.iterateStereocenters()):
    s.setStereocenterGroup(2)
    print("  %d: %d" % (s.index(), s.stereocenterGroup()))
print(m.canonicalSmiles())

print("****** Mark stereobonds ********")
m = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/stereo-many.mol", __file__)
)
print(m.molfile())
m.saveMolfile(joinPathPy("out/stereo-many-original.mol", __file__))
m.markStereobonds()
print(m.molfile())
m.saveMolfile(joinPathPy("out/stereo-many-marked.mol", __file__))

print("****** Validate chirality ********")
mols = [
    "CC1C[C@@H](C)C[C@H](C)C1",
    "C[C@H]1C[C@H](C)[C@H](C)C[C@H]1C",
    "CCCCCCCC[C@H](C)C",
]
for mstr in mols:
    print(mstr)
    m = indigo.loadMolecule(mstr)
    print("  Cano:   " + m.canonicalSmiles())
    print("  Smiles: " + m.smiles())
    print("  Validating...")
    m.validateChirality()
    print("  Cano:   " + m.canonicalSmiles())
    print("  Smiles: " + m.smiles())


print("****** Stereocenters in SMILES and Molfile ********")
mabs = indigo.loadMolecule("N[C@@H]1C[C@@H](C)[C@@H](C)C[C@@H]1C")
mand = indigo.loadMolecule("N[C@@H]1C[C@@H](C)[C@@H](C)C[C@@H]1C |&1:1,3,5,8|")
mor = indigo.loadMolecule("N[C@@H]1C[C@@H](C)[C@@H](C)C[C@@H]1C |o1:1,3,5,8|")

for m in [mabs, mand, mor]:
    m.layout()
    print(m.smiles())
    m.setProperty("original-smiles", m.smiles())

    m2 = indigo.loadMolecule(m.molfile())
    print(m2.smiles())

output = indigo.createFileSaver(
    joinPathPy("out/stereosmiles.sdf", __file__), "sdf"
)

for format in ["2000", "3000"]:
    indigo.setOption("molfile-saving-mode", format)
    for m in [mabs, mand, mor]:
        m.setProperty("format", format)
        m2 = indigo.loadMolecule(m.molfile())
        m.setProperty("reloaded-smiles", m2.smiles())
        output.append(m)
