"""
The charge and the radical of an attachment group, drawn by the centre of the
group.

Every picture is compared as SVG: the text of the drawing shows which sign is
drawn and where it stands, which the coarse hash of a PNG does not.
"""

# The Java job runs this suite under Jython 2.7, where a bare print() would print
# an empty tuple instead of an empty line.
from __future__ import print_function

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


def load(fixture):
    return indigo.loadMoleculeFromFile(
        os.path.join(molecules_path, fixture + ".ket")
    )


def svg(picture):
    return "".join(chr(b & 0xFF) for b in renderer.renderToBuffer(picture))


def compare(name, picture_svg):
    compare_diff(ref_path, name + ".svg", picture_svg)


def without_charges(molecule):
    for group in molecule.iterateAttachmentGroups():
        group.setCharge(0)
        group.setRadical(0)
    return molecule


print("*** The corner of each quadrant, and the boundaries between them ***")
for name in (
    "ring-bond-up",
    "ring-bond-right",
    "ring-bond-down",
    "ring-bond-left",
    "benzene-bond-45",
    "benzene-bond-135",
    "benzene-bond-225",
    "benzene-bond-315",
):
    compare(name, svg(load(name)))

print("*** The radical beside the charge ***")
for name in ("ring-doublet", "ring-singlet", "ring-triplet"):
    compare(name, svg(load(name)))

print("*** Out of the corner when something is in the way ***")
for name in (
    "ring-bond-60",
    "ring-bond-120",
    "ethylene-bond-left",
    "ethylene-bond-40",
    "ethylene-askew",
    "allyl-bond-up",
    "allyl-bond-75",
    "allyl-bond-75-radical",
    "allyl-bond-120",
):
    compare(name, svg(load(name)))

print("*** The first haptic bond decides; sandwiches ***")
for name in ("ring-two-metals", "sandwich-vertical", "sandwich-horizontal"):
    compare(name, svg(load(name)))

print("*** No label ***")
compare("ring-neutral", svg(load("ring-neutral")))
# A group no haptic bond refers to is drawn as if it carried nothing.
unbonded = load("ring-without-haptic-bond")
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

print("*** A reaction and a grid ***")
first, second = "ring-bond-up", "ring-bond-right"
reaction = indigo.createReaction()
reaction.addReactant(load(first))
reaction.addProduct(load(second))
compare(first + "-reaction", svg(reaction))
grid = indigo.createArray()
grid.arrayAdd(load(first))
grid.arrayAdd(load(second))
compare(
    first + "-grid",
    "".join(chr(b & 0xFF) for b in renderer.renderGridToBuffer(grid, None, 2)),
)

print("*** PNG ***")
# The hash of the PNG check does not see a label this small; the picture
# itself is compared with the one of the same structure without charges.
indigo.setOption("render-output-format", "png")
png_fixture = "sandwich-vertical"
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
