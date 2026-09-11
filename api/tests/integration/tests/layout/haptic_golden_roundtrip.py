import math
import os
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), "..", "..", "common"))
from env_indigo import *  # noqa

# The golden complexes driven through the public API the way a caller uses it:
# loaded from a file, laid out, saved, loaded back, converted to MOL V3000 and
# rendered. haptic_golden.py checks the geometry the layout produces; this one
# checks that the geometry survives the wrappers and the formats around it.
#
# The cases are one of each shape the set contains: a sandwich, a chelating
# ligand, a bridging one, an eta-2 end with a stereocentre on the metal, and the
# single atom-to-atom bond.

CASES = [
    ("ferrocene.ket", 2, 2),
    ("bis-1-5-cyclooctadiene-nickel-0.ket", 4, 4),
    ("tris-cyclooctatetraene-triiron.ket", 6, 6),
    ("trichloro-ethylene-platinate-ii-hydrate-zeises-salt.ket", 1, 1),
    ("trans-ir-co-pph3-2-indolyl.ket", 0, 1),
    ("second-complex-from-lu-et-al-2015.ket", 8, 8),
]

TOLERANCE = 0.02

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)
# V2000 has no way to express a haptic bond, so the V3000 mode is not optional here.
indigo.setOption("molfile-saving-mode", "3000")
renderer = IndigoRenderer(indigo)
indigo.setOption("render-output-format", "svg")

folder = joinPathPy("haptic-golden", __file__)


def centroid(points):
    return (
        sum(p[0] for p in points) / len(points),
        sum(p[1] for p in points) / len(points),
    )


def haptic_lengths(molecule):
    """Length of every haptic bond, measured from the centroid of a group end."""
    positions = {a.index(): a.xyz()[:2] for a in molecule.iterateAtoms()}
    lengths = []
    for bond in molecule.iterateHapticBonds():
        ends = []
        for endpoint in (bond.hapticBondBegin(), bond.hapticBondEnd()):
            if endpoint.isAttachmentGroup():
                ends.append(
                    centroid(
                        [positions[a.index()] for a in endpoint.iterateAtoms()]
                    )
                )
            else:
                ends.append(positions[endpoint.index()])
        lengths.append(
            math.hypot(ends[0][0] - ends[1][0], ends[0][1] - ends[1][1])
        )
    return sorted(lengths)


for name, expected_groups, expected_bonds in CASES:
    path = os.path.join(folder, name)

    # 1. the file loader of the wrapper, not a string handed over by the test
    molecule = indigo.loadMoleculeFromFile(path)
    groups = len(list(molecule.iterateAttachmentGroups()))
    bonds = len(list(molecule.iterateHapticBonds()))
    print("%s: loaded %d group(s), %d haptic bond(s)" % (name, groups, bonds))
    if (groups, bonds) != (expected_groups, expected_bonds):
        print(
            "  UNEXPECTED: expected %d group(s) and %d bond(s)"
            % (expected_groups, expected_bonds)
        )

    molecule.layout()
    lengths = haptic_lengths(molecule)

    # 2. KET round trip: the layout result has to survive saving and loading
    reloaded = indigo.loadMolecule(molecule.json())
    reloaded_groups = len(list(reloaded.iterateAttachmentGroups()))
    reloaded_bonds = len(list(reloaded.iterateHapticBonds()))
    if (reloaded_groups, reloaded_bonds) != (groups, bonds):
        print(
            "  KET round trip lost data: %d group(s), %d bond(s)"
            % (reloaded_groups, reloaded_bonds)
        )
    else:
        print(
            "  KET round trip keeps %d group(s) and %d bond(s)"
            % (reloaded_groups, reloaded_bonds)
        )

    reloaded_lengths = haptic_lengths(reloaded)
    drift = max(
        (abs(a - b) for a, b in zip(lengths, reloaded_lengths)), default=0.0
    )
    print(
        "  bond lengths after a round trip differ by at most %.3f" % drift
        if drift > TOLERANCE
        else "  bond lengths unchanged by the round trip"
    )

    # 3. MOL V3000: a group-to-atom bond becomes an ENDPTS record, an atom-to-atom
    # one has no representation there and is dropped - the documented limit.
    molfile = molecule.molfile()
    records = molfile.count("ENDPTS=")
    print("  MOL V3000 carries %d ENDPTS record(s)" % records)

    if records:
        from_molfile = indigo.loadMolecule(molfile)
        print(
            "  reloaded from MOL V3000: %d group(s), %d haptic bond(s)"
            % (
                len(list(from_molfile.iterateAttachmentGroups())),
                len(list(from_molfile.iterateHapticBonds())),
            )
        )

    # 4. the renderer draws one line per haptic bond on top of the ordinary ones
    svg = bytes(renderer.renderToBuffer(molecule)).decode("utf-8")
    print("  rendered, %d path element(s)" % svg.count("<path"))
