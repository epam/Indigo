"""
The charge and the radical of an attachment group, drawn by the centre of the
group.

Every picture is compared as SVG: the text of the drawing shows which sign is
drawn and where it stands, which the coarse hash of a PNG does not. The cases
of a section share one picture, a grid with the name of each case under it. A
ligand is one fixture, turned to the direction of each case; the charge and
the radical of a case are set through the API.
"""

# The Java job runs this suite under Jython 2.7, where a bare print() would print
# an empty tuple instead of an empty line.
from __future__ import print_function

import math
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff, isIronPython  # noqa
from env_indigo import Indigo, IndigoRenderer, joinPathPy  # noqa

SUBJECT = "haptic_ligand_charge"
molecules_path = joinPathPy("molecules/%s/" % SUBJECT, __file__)
ref_path = joinPathPy("ref/%s/" % SUBJECT, __file__)

indigo = Indigo()
renderer = IndigoRenderer(indigo)
# No render-fonts: the bundled font draws the same glyphs on every platform, so
# one reference serves them all.
indigo.resetOptions()
indigo.setOption("render-output-format", "svg")
indigo.setOption("render-svg-unique-id", False)
GRID_TITLE = "grid-comment"
indigo.setOption("render-grid-title-property", GRID_TITLE)
indigo.setOption("render-grid-margins", "40, 40")
indigo.setOption("render-grid-title-offset", 10)
# The ring drawn in perspective has wedges on two of its bonds, which the stereo
# check refuses.
indigo.setOption("ignore-stereochemistry-errors", True)


# Directions in degrees counterclockwise from the x axis.
UP, RIGHT, DOWN, LEFT = 90, 0, 270, 180
AXES = (UP, RIGHT, DOWN, LEFT)
AXIS_NAMES = {UP: "up", RIGHT: "right", DOWN: "down", LEFT: "left"}
DIAGONALS = (45, 135, 225, 315)
RING, BENZENE, ETHYLENE, ALLYL = "ring", "benzene", "ethylene", "allyl"
# The fixture of each ligand and the direction of its haptic bond there.
LIGANDS = {RING: UP, BENZENE: 45, ETHYLENE: LEFT, ALLYL: UP}
DECIMALS = 3  # of the coordinates in the fixtures
SANDWICH = "sandwich-vertical"
ALLYL_LINE_NEAR_BOND = 75


def load(fixture):
    return indigo.loadMoleculeFromFile(
        os.path.join(molecules_path, fixture + ".ket")
    )


def turned(fixture, degrees):
    """The fixture turned counterclockwise about the origin. The coordinates
    are rounded as in the fixtures, so the sine and cosine of every runtime
    give the same picture."""
    molecule = load(fixture)
    angle = math.radians(degrees)
    cos, sin = math.cos(angle), math.sin(angle)
    for atom in molecule.iterateAtoms():
        xyz = atom.xyz()
        x, y = float(xyz[0]), float(xyz[1])
        atom.setXYZ(
            round(cos * x - sin * y, DECIMALS),
            round(sin * x + cos * y, DECIMALS),
            float(xyz[2]),
        )
    return molecule


def bonded(ligand, degrees):
    return turned(ligand, degrees - LIGANDS[ligand])


def labelled(molecule, charge, radical):
    group = molecule.getAttachmentGroup(0)
    group.setCharge(charge)
    group.setRadical(radical)
    return molecule


def oriented(ligand, degrees, charge=None):
    """A case: its name and a function that makes the ligand with its haptic
    bond at `degrees`, with `charge` instead of the fixture's when given."""

    def make():
        molecule = bonded(ligand, degrees)
        if charge is not None:
            molecule.getAttachmentGroup(0).setCharge(charge)
        return molecule

    return "%s-bond-%s" % (ligand, AXIS_NAMES.get(degrees, degrees)), make


def load_query(fixture):
    return indigo.loadQueryMoleculeFromFile(
        os.path.join(molecules_path, fixture + ".mol")
    )


def as_drawn(fixture, loader=load):
    """A case drawn from its fixture as it is."""
    return fixture, lambda: loader(fixture)


def svg(picture):
    return "".join(chr(b & 0xFF) for b in renderer.renderToBuffer(picture))


def grid_svg(cells, columns=None):
    """One picture of the cells, each a (title, molecule), `columns` a row;
    all of them in one row by default."""
    grid = indigo.createArray()
    for title, molecule in cells:
        molecule.setProperty(GRID_TITLE, title)
        grid.arrayAdd(molecule)
    return "".join(
        chr(b & 0xFF)
        for b in renderer.renderGridToBuffer(grid, None, columns or len(cells))
    )


def compare(name, picture_svg):
    compare_diff(ref_path, name + ".svg", picture_svg)


def without_charges(molecule):
    for group in molecule.iterateAttachmentGroups():
        group.setCharge(0)
        group.setRadical(0)
    return molecule


DIANION = -2
DIANION_LABELS = (
    ("", 0),
    (" singlet", Indigo.SINGLET),
    (" triplet", Indigo.TRIPLET),
)
DIANION_COLUMNS = 2 * len(DIANION_LABELS)  # two cases a row


def dianion_cells(cases):
    return [
        (name + suffix, labelled(make(), DIANION, radical))
        for name, make in cases
        for suffix, radical in DIANION_LABELS
    ]


def drawn(cases):
    return [(name, make()) for name, make in cases]


RING_AXIS_CHARGES = ((UP, -1), (RIGHT, -2), (DOWN, 1), (LEFT, 12))
RING_CASES = [
    oriented(RING, degrees, charge) for degrees, charge in RING_AXIS_CHARGES
] + [oriented(BENZENE, degrees) for degrees in DIAGONALS]
ETHYLENE_CASES = [oriented(ETHYLENE, degrees) for degrees in AXES + DIAGONALS]
ALLYL_CASES = [oriented(ALLYL, degrees) for degrees in AXES + DIAGONALS]
ALLYLS_IN_THE_WAY = [
    oriented(ALLYL, ALLYL_LINE_NEAR_BOND),
    oriented(ALLYL, 120),
    as_drawn("allyl-convex-radical"),
]
IN_THE_WAY = "in-the-way"

print("*** The four directions of the bond, and the diagonals ***")
compare(
    "directions",
    grid_svg(
        drawn(RING_CASES + ETHYLENE_CASES + ALLYL_CASES),
        len(AXES + DIAGONALS),
    ),
)

print("*** The radical beside the charge ***")
compare(
    "radical-beside-charge",
    grid_svg(
        [
            (name, labelled(bonded(RING, degrees), charge, radical))
            for name, degrees, charge, radical in (
                ("ring-doublet", UP, 0, Indigo.DOUBLET),
                ("ring-singlet", RIGHT, -1, Indigo.SINGLET),
                ("ring-triplet", DOWN, -2, Indigo.TRIPLET),
            )
        ]
    ),
)

print("*** Along the line when something is in the way ***")
compare(
    IN_THE_WAY,
    grid_svg(
        drawn(
            [
                as_drawn("ring-bond-60"),
                as_drawn("ring-bond-120"),
                as_drawn("ring-perspective"),
                as_drawn("cyclopropenyl-aromatic"),
                as_drawn("benzene-eta4-aromatic"),
                oriented(ETHYLENE, 40),
                as_drawn("ethylene-askew"),
                as_drawn("ethylene-single-or-double", load_query),
                (
                    "allyl-bond-%d-radical" % ALLYL_LINE_NEAR_BOND,
                    lambda: labelled(
                        bonded(ALLYL, ALLYL_LINE_NEAR_BOND),
                        -1,
                        Indigo.DOUBLET,
                    ),
                ),
            ]
            + ALLYLS_IN_THE_WAY
        ),
        4,
    ),
)

print("*** A dianion, alone and with a diradical ***")
for group, cases in (
    (RING, RING_CASES),
    (ETHYLENE, ETHYLENE_CASES),
    (ALLYL, ALLYL_CASES),
    (IN_THE_WAY, ALLYLS_IN_THE_WAY),
):
    compare(
        "dianion-" + group, grid_svg(dianion_cells(cases), DIANION_COLUMNS)
    )

print("*** The first haptic bond decides; sandwiches ***")
compare(
    "sandwiches",
    grid_svg(
        drawn(
            [
                as_drawn("ring-two-metals"),
                as_drawn(SANDWICH),
                ("sandwich-horizontal", lambda: turned(SANDWICH, 90)),
            ]
        )
    ),
)

print("*** No label ***")
compare("ring-neutral", svg(labelled(bonded(RING, UP), 0, 0)))
# A group no haptic bond refers to is drawn as if it carried nothing.
unbonded = load(RING)
unbonded.getHapticBond(0).remove()
unbonded_charged = svg(unbonded)
print(
    "ring-without-haptic-bond drawn as without the charge: %s"
    % (unbonded_charged == svg(without_charges(unbonded)))
)
# No file gives a variable attachment a charge; only the API does.
markush = indigo.loadMoleculeFromFile(
    os.path.join(molecules_path, "variable-attachment.mol")
)
markush_neutral = svg(markush)
markush.getAttachmentGroup(0).setCharge(-1)
print(
    "variable-attachment drawn as without the charge: %s"
    % (markush_neutral == svg(markush))
)

print("*** A picture of its own and a reaction ***")
(first, make_first), (second, make_second) = RING_CASES[:2]
compare(first, svg(make_first()))
reaction = indigo.createReaction()
reaction.addReactant(make_first())
reaction.addProduct(make_second())
compare(first + "-reaction", svg(reaction))

print("*** PNG ***")
# The hash of the PNG check does not see a label this small; the picture
# itself is compared with the one of the same structure without charges.
indigo.setOption("render-output-format", "png")
png_fixture = SANDWICH
sandwich = load(png_fixture)
sandwich_charged = renderer.renderToBuffer(sandwich)
print(
    "%s PNG differs from the one without charges: %s"
    % (
        png_fixture,
        list(sandwich_charged)
        != list(renderer.renderToBuffer(without_charges(sandwich))),
    )
)

if isIronPython():
    renderer.Dispose()
    indigo.Dispose()
