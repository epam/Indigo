import math
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

# What a drawing may not show, counted rather than looked at: two bonds that
# cross, an atom on a bond it has nothing to do with, two atoms in one place. A
# haptic bond is not an edge, so it never appears here - it is allowed to cross.
# Collisions inside one component belong to the ordinary layout; the ones between
# components are what the haptic placement is responsible for, so they are
# reported apart.
#
# The lengths of the haptic bonds are printed beside them, because they are the
# requirement (#3233, 4) and because neither they nor the counts are coordinates:
# they survive a layout that comes out mirrored, turned or in another atom order,
# which is what makes this file the one test of the set that needs no per-platform
# reference.
#
# They are printed to three decimals, not two: a stretched bond is 1.5 times a
# stretch factor, and 1.5 * 1.25 = 1.875 lies exactly on a rounding boundary
# of the second decimal, where float noise decides the digit.

BUMP = 0.45  # two atom labels this close are drawn on top of each other
ON_BOND = 0.3  # a bond is a thin line and needs less room than a label

indigo = Indigo()

root = joinPathPy("molecules/", __file__)

files = [
    "allylpalladium-chloride-dimer",
    "bis-1-5-cyclooctadiene-nickel-0",
    "bis-benzene-chromium",
    "bis-cyclopentadienyl-dimethylzirconium-iv",
    "complex-from-kaufmann-et-al-2023",
    "complex-from-seyferth-2002",
    "complex-from-wetzel-roesky-1998",
    "cycloheptatrienyl-cyclopentadienyl-titanium",
    "cycloheptatrienyl-molybdenum-tricarbonyl",
    "cyclobutadiene-iron-tricarbonyl",
    "cyclooctadiene-rhodium-chloride-dimer",
    "cyclopentadienyl-iron-dicarbonyl-dimer",
    "cyclopentadienyl-manganese-tricarbonyl",
    "eta3-allyl-eta5-cyclopentadienyl-palladium",
    "ferrocene",
    "first-complex-from-lu-et-al-2015",
    "pdcl2-bis-diphenylphosphanyl-ferrocene-2",
    "second-complex-from-lu-et-al-2015",
    "tetra-cyclopentadienyl-titanium",
    "third-complex-from-lu-et-al-2015",
    "trans-ir-co-pph3-2-indolyl",
    "tricarbonyl-benzene-chromium",
    "trichloro-ethylene-platinate-ii-hydrate-zeises-salt",
    "triple-decker-complex-from-ghag-at-el-2013",
    "tris-cyclooctatetraene-triiron",
    "tris-cyclopentadienyl-yttrium-iii",
    "uranocene",
]


def turn(origin, first, second):
    return (first[0] - origin[0]) * (second[1] - origin[1]) - (
        first[1] - origin[1]
    ) * (second[0] - origin[0])


def bonds_cross(p, q, r, s):
    """True when the two segments properly cross, touching not counted."""
    first = turn(r, s, p) * turn(r, s, q)
    second = turn(p, q, r) * turn(p, q, s)
    return first < 0 and second < 0


def distance_to_bond(point, begin, end):
    dx, dy = end[0] - begin[0], end[1] - begin[1]
    length = dx * dx + dy * dy
    if length < 1e-12:
        return math.hypot(point[0] - begin[0], point[1] - begin[1])
    along = ((point[0] - begin[0]) * dx + (point[1] - begin[1]) * dy) / length
    along = max(0.0, min(1.0, along))
    return math.hypot(
        point[0] - begin[0] - along * dx, point[1] - begin[1] - along * dy
    )


def geometry(molecule):
    component_of = {}
    for number, component in enumerate(molecule.iterateComponents()):
        for atom in component.iterateAtoms():
            component_of[atom.index()] = number

    # The .NET wrapper hands the coordinates over as System.Single, and
    # IronPython keeps single precision through the arithmetic unless they are
    # widened here.
    position = {}
    for atom in molecule.iterateAtoms():
        xyz = atom.xyz()
        position[atom.index()] = (float(xyz[0]), float(xyz[1]))

    bonds = []
    for bond in molecule.iterateBonds():
        bonds.append((bond.source().index(), bond.destination().index()))

    return component_of, position, bonds


def count(molecule):
    component_of, position, bonds = geometry(molecule)
    inside = [0, 0, 0]
    between = [0, 0, 0]

    def bucket(one, two):
        return inside if component_of[one] == component_of[two] else between

    for i in range(len(bonds)):
        for j in range(i + 1, len(bonds)):
            a, b = bonds[i]
            c, d = bonds[j]
            if len(set([a, b, c, d])) < 4:
                continue
            if bonds_cross(position[a], position[b], position[c], position[d]):
                bucket(a, c)[0] += 1

    bonded = set()
    for a, b in bonds:
        bonded.add((a, b))
        bonded.add((b, a))

    atoms = sorted(position)
    for index, a in enumerate(atoms):
        for b in atoms[index + 1 :]:
            if (a, b) in bonded:
                continue
            distance = math.hypot(
                position[a][0] - position[b][0],
                position[a][1] - position[b][1],
            )
            if distance < BUMP:
                bucket(a, b)[1] += 1

    for atom in atoms:
        for a, b in bonds:
            if atom == a or atom == b:
                continue
            if (
                distance_to_bond(position[atom], position[a], position[b])
                < ON_BOND
            ):
                bucket(atom, a)[2] += 1

    return inside, between


def haptic_lengths(molecule):
    """From the centre of a group end, which is where the bond starts (N5)."""
    _, position, _ = geometry(molecule)

    lengths = []
    for bond in molecule.iterateHapticBonds():
        ends = []
        for endpoint in (bond.hapticBondBegin(), bond.hapticBondEnd()):
            if endpoint.isAttachmentGroup():
                atoms = [a.index() for a in endpoint.iterateAtoms()]
            else:
                atoms = [endpoint.index()]
            ends.append(
                (
                    sum(position[a][0] for a in atoms) / len(atoms),
                    sum(position[a][1] for a in atoms) / len(atoms),
                )
            )
        lengths.append(
            math.hypot(ends[0][0] - ends[1][0], ends[0][1] - ends[1][1])
        )

    return sorted(lengths)


print("*** Haptic bond layout: crossings, bumps and bond lengths ***")

files.sort()
for filename in files:
    molecule = indigo.loadMoleculeFromFile(
        os.path.join(root, filename + ".ket")
    )
    molecule.layout()
    inside, between = count(molecule)
    print(
        "%-52s between %d/%d/%d inside %d/%d/%d lengths %s"
        % (
            filename,
            between[0],
            between[1],
            between[2],
            inside[0],
            inside[1],
            inside[2],
            " ".join("%.3f" % value for value in haptic_lengths(molecule)),
        )
    )
