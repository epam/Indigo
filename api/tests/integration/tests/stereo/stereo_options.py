import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", False)
indigo.setOption("molfile-saving-skip-date", "1")

print("****** Load/save with default options  *******")
set = [
    m
    for m in indigo.iterateSDFile(
        joinPathPy("molecules/chiral_test.sdf", __file__)
    )
]
for m in set:
    print("%s. Chiral: %s" % (m.canonicalSmiles(), m.isChiral()))
    for s in m.iterateStereocenters():
        print("  %d: %d" % (s.index(), s.stereocenterType()))


print("****** Load/save with treat-stereo-as options  *******")
print("****** abs *******")
indigo.setOption("treat-stereo-as", "abs")
set = [
    m
    for m in indigo.iterateSDFile(
        joinPathPy("molecules/chiral_test.sdf", __file__)
    )
]
for m in set:
    print("%s. Chiral: %s" % (m.canonicalSmiles(), m.isChiral()))
    for s in m.iterateStereocenters():
        print("  %d: %d" % (s.index(), s.stereocenterType()))

print("****** rel *******")
indigo.setOption("treat-stereo-as", "rel")
set = [
    m
    for m in indigo.iterateSDFile(
        joinPathPy("molecules/chiral_test.sdf", __file__)
    )
]
for m in set:
    print("%s. Chiral: %s" % (m.canonicalSmiles(), m.isChiral()))
    for s in m.iterateStereocenters():
        print("  %d: %d" % (s.index(), s.stereocenterType()))


print("****** rac *******")
indigo.setOption("treat-stereo-as", "rac")
set = [
    m
    for m in indigo.iterateSDFile(
        joinPathPy("molecules/chiral_test.sdf", __file__)
    )
]
for m in set:
    print("%s. Chiral: %s" % (m.canonicalSmiles(), m.isChiral()))
    for s in m.iterateStereocenters():
        print("  %d: %d" % (s.index(), s.stereocenterType()))


print("****** any *******")
indigo.setOption("treat-stereo-as", "any")
set = [
    m
    for m in indigo.iterateSDFile(
        joinPathPy("molecules/chiral_test.sdf", __file__)
    )
]
for m in set:
    print("%s. Chiral: %s" % (m.canonicalSmiles(), m.isChiral()))
    for s in m.iterateStereocenters():
        print("  %d: %d" % (s.index(), s.stereocenterType()))


print("****** Load/save with chiral_flag option  *******")
print("****** default *******")
indigo.setOption("treat-stereo-as", "ucf")
indigo.setOption("molfile-saving-chiral-flag", -1)
set = [
    m
    for m in indigo.iterateSDFile(
        joinPathPy("molecules/chiral_test.sdf", __file__)
    )
]
for m in set:
    print("%s. Chiral: %s" % (m.canonicalSmiles(), m.isChiral()))
    for s in m.iterateStereocenters():
        print("  %d: %d" % (s.index(), s.stereocenterType()))
    print(m.molfile())

print("****** no chiral flag *******")
indigo.setOption("molfile-saving-chiral-flag", 0)
set = [
    m
    for m in indigo.iterateSDFile(
        joinPathPy("molecules/chiral_test.sdf", __file__)
    )
]
for m in set:
    print("%s. Chiral: %s" % (m.canonicalSmiles(), m.isChiral()))
    for s in m.iterateStereocenters():
        print("  %d: %d" % (s.index(), s.stereocenterType()))
    print(m.molfile())


print("****** chiral flag *******")
indigo.setOption("molfile-saving-chiral-flag", 1)
set = [
    m
    for m in indigo.iterateSDFile(
        joinPathPy("molecules/chiral_test.sdf", __file__)
    )
]
for m in set:
    print("%s. Chiral: %s" % (m.canonicalSmiles(), m.isChiral()))
    for s in m.iterateStereocenters():
        print("  %d: %d" % (s.index(), s.stereocenterType()))
    print(m.molfile())


print("****** Extended smiles options  *******")

# When ignore-no-chiral-flag is set up to True then all stereochemictry centers,
# whose type are not specified become ABS
indigo.setOption("ignore-no-chiral-flag", False)

# test 1
m = indigo.loadMolecule(
    "C[C@@H]1[C@H](C)C(C2O[C@](F)(C)[C@]2(C)C)CN1 |&1:1,a:2,o1:7,10,r|"
)
for atom in m.iterateStereocenters():
    print(
        "atom index = {} stereo = {}".format(
            atom.index(), atom.stereocenterType()
        )
    )
assert m.getAtom(1).stereocenterType() == indigo.AND
assert m.getAtom(2).stereocenterType() == indigo.ABS
assert m.getAtom(7).stereocenterType() == indigo.OR
assert m.getAtom(10).stereocenterType() == indigo.OR

# test 2
m = indigo.loadMolecule(
    "C[C@@H]1[C@H](C)C(C2O[C@](F)(C)[C@]2(C)C)CN1 |&1:1,a:2,o1:7,10|"
)
for atom in m.iterateStereocenters():
    print(
        "atom index = {} stereo = {}".format(
            atom.index(), atom.stereocenterType()
        )
    )
assert m.getAtom(1).stereocenterType() == indigo.AND
assert m.getAtom(2).stereocenterType() == indigo.ABS
assert m.getAtom(7).stereocenterType() == indigo.OR
assert m.getAtom(10).stereocenterType() == indigo.OR

# test 3
m = indigo.loadMolecule(
    "C[C@@H]1[C@H](C)C(C2O[C@](F)(C)[C@]2(C)C)CN1 |&1:1,o1:7,10,r|"
)
for atom in m.iterateStereocenters():
    print(
        "atom index = {} stereo = {}".format(
            atom.index(), atom.stereocenterType()
        )
    )
assert m.getAtom(1).stereocenterType() == indigo.AND
assert m.getAtom(2).stereocenterType() == indigo.AND
assert m.getAtom(7).stereocenterType() == indigo.OR
assert m.getAtom(10).stereocenterType() == indigo.OR

# test 4
m = indigo.loadMolecule(
    "C[C@@H]1[C@H](C)C(C2O[C@](F)(C)[C@]2(C)C)CN1 |&1:1,o1:7,10|"
)
for atom in m.iterateStereocenters():
    print(
        "atom index = {} stereo = {}".format(
            atom.index(), atom.stereocenterType()
        )
    )
assert m.getAtom(1).stereocenterType() == indigo.AND
assert m.getAtom(2).stereocenterType() == indigo.ABS
assert m.getAtom(7).stereocenterType() == indigo.OR
assert m.getAtom(10).stereocenterType() == indigo.OR

indigo.setOption("ignore-no-chiral-flag", True)
# test 5
m = indigo.loadMolecule(
    "C[C@@H]1[C@H](C)C(C2O[C@](F)(C)[C@]2(C)C)CN1 |&1:1,a:2,o1:7,10,r|"
)
for atom in m.iterateStereocenters():
    print(
        "atom index = {} stereo = {}".format(
            atom.index(), atom.stereocenterType()
        )
    )
assert m.getAtom(1).stereocenterType() == indigo.AND
assert m.getAtom(2).stereocenterType() == indigo.ABS
assert m.getAtom(7).stereocenterType() == indigo.OR
assert m.getAtom(10).stereocenterType() == indigo.OR

# test 6
m = indigo.loadMolecule(
    "C[C@@H]1[C@H](C)C(C2O[C@](F)(C)[C@]2(C)C)CN1 |&1:1,a:2,o1:7,10|"
)
for atom in m.iterateStereocenters():
    print(
        "atom index = {} stereo = {}".format(
            atom.index(), atom.stereocenterType()
        )
    )
assert m.getAtom(1).stereocenterType() == indigo.AND
assert m.getAtom(2).stereocenterType() == indigo.ABS
assert m.getAtom(7).stereocenterType() == indigo.OR
assert m.getAtom(10).stereocenterType() == indigo.OR

# test 7
m = indigo.loadMolecule(
    "C[C@@H]1[C@H](C)C(C2O[C@](F)(C)[C@]2(C)C)CN1 |&1:1,o1:7,10,r|"
)
for atom in m.iterateStereocenters():
    print(
        "atom index = {} stereo = {}".format(
            atom.index(), atom.stereocenterType()
        )
    )
assert m.getAtom(1).stereocenterType() == indigo.AND
assert m.getAtom(2).stereocenterType() == indigo.ABS
assert m.getAtom(7).stereocenterType() == indigo.OR
assert m.getAtom(10).stereocenterType() == indigo.OR

# test 8
m = indigo.loadMolecule(
    "C[C@@H]1[C@H](C)C(C2O[C@](F)(C)[C@]2(C)C)CN1 |&1:1,o1:7,10|"
)
for atom in m.iterateStereocenters():
    print(
        "atom index = {} stereo = {}".format(
            atom.index(), atom.stereocenterType()
        )
    )
assert m.getAtom(1).stereocenterType() == indigo.AND
assert m.getAtom(2).stereocenterType() == indigo.ABS
assert m.getAtom(7).stereocenterType() == indigo.OR
assert m.getAtom(10).stereocenterType() == indigo.OR
