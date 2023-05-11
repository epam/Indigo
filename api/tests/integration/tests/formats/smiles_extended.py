import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)

from indigo import Indigo

indigo = Indigo()

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
