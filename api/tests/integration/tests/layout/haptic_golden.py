import errno
import math
import os
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), "..", "..", "common"))
from env_indigo import *  # noqa

# The golden set: real organometallics collected by the chemist for #3233, each a
# haptic complex Ketcher produced. Hapticity runs from eta-2 to eta-8; some are
# sandwiches, some chelating, some bridge two metals. Every one of them says the
# same thing about the layout, so they are checked by rule rather than by
# reference coordinates - coordinates differ between platforms (about 12% of the
# corpus does), the geometry does not.
#
# The rules, from requirement 4 of #3233 and the conditions of the chemist:
#   * a group-to-atom haptic bond is 1.5 standard bond lengths, measured from the
#     centroid of the participating atoms;
#   * an atom-to-atom one keeps the ordinary length;
#   * a bond leaving a two- or three-atom group crosses it at a right angle,
#     rather than running along the ligand's own bonds - which is possible only
#     when that group holds one bond. A chelating or bridging ligand is held by
#     several at once, and equal lengths and right angles cannot both be had:
#     the length wins, and the angle is whatever the fit leaves.

BOND_LENGTH = 1.0
GROUP_MULTIPLIER = 1.5
TOLERANCE = 0.02
ANGLE_TOLERANCE = 5.0

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)

folder = joinPathPy("haptic-golden", __file__)


def centroid(points):
    return (sum(p[0] for p in points) / len(points), sum(p[1] for p in points) / len(points))


def distance(a, b):
    return math.hypot(a[0] - b[0], a[1] - b[1])


def endpoint_geometry(endpoint, positions):
    """The point a bond ends at, and the atoms of that end."""
    if endpoint.isAttachmentGroup():
        members = [a.index() for a in endpoint.iterateAtoms()]
        return centroid([positions[i] for i in members]), members
    return positions[endpoint.index()], [endpoint.index()]


def right_angle_error(members, positions, point):
    """How far from perpendicular the bond meets a two- or three-atom group.

    Such a group is a line: the bond has to cross it, not run along it. Returns
    None when the group is a ring, which has no axis to be perpendicular to.
    """
    if len(members) < 2 or len(members) > 3:
        return None

    # The major axis of the atoms: the order they are listed in is the file's, not
    # the geometry's - an allyl group may well start at its middle atom.
    centre_of_group = centroid([positions[i] for i in members])
    xx = sum((positions[i][0] - centre_of_group[0]) ** 2 for i in members)
    xy = sum((positions[i][0] - centre_of_group[0]) * (positions[i][1] - centre_of_group[1]) for i in members)
    yy = sum((positions[i][1] - centre_of_group[1]) ** 2 for i in members)
    delta = math.sqrt(max(0.0, (xx - yy) ** 2 + 4 * xy * xy))
    major = (xx + yy + delta) / 2
    if major < 1e-8:
        return None
    axis = (major - yy, xy) if abs(xy) > 1e-8 else ((1.0, 0.0) if xx >= yy else (0.0, 1.0))
    if math.hypot(*axis) < 1e-4:
        return None

    centre = centroid([positions[i] for i in members])
    bond = (point[0] - centre[0], point[1] - centre[1])
    if math.hypot(*bond) < 1e-4:
        return None

    cosine = (axis[0] * bond[0] + axis[1] * bond[1]) / (math.hypot(*axis) * math.hypot(*bond))
    return abs(90.0 - math.degrees(math.acos(max(-1.0, min(1.0, cosine)))))


failures = 0
for name in sorted(os.listdir(folder)):
    if not name.endswith(".ket"):
        continue

    with open(os.path.join(folder, name), encoding="utf-8") as source:
        molecule = indigo.loadMolecule(source.read())

    molecule.layout()
    positions = {a.index(): a.xyz()[:2] for a in molecule.iterateAtoms()}

    component_of = {}
    for number, component in enumerate(molecule.iterateComponents()):
        for atom in component.iterateAtoms():
            component_of[atom.index()] = number

    # How many haptic bonds hold each component. The right angle is asked only
    # where a component is held by one of them: a ligand held by two - chelating
    # like cyclooctadiene, or bridging two metals like cyclooctatetraene here -
    # cannot give both bonds the right length and both groups a right angle, and
    # the length is the requirement.
    held = {}
    for bond in molecule.iterateHapticBonds():
        for endpoint in (bond.hapticBondBegin(), bond.hapticBondEnd()):
            atoms = [a.index() for a in endpoint.iterateAtoms()] if endpoint.isAttachmentGroup() else [endpoint.index()]
            component = component_of.get(atoms[0])
            held[component] = held.get(component, 0) + 1

    problems = []
    bonds = 0
    for bond in molecule.iterateHapticBonds():
        bonds += 1
        begin, begin_atoms = endpoint_geometry(bond.hapticBondBegin(), positions)
        end, end_atoms = endpoint_geometry(bond.hapticBondEnd(), positions)

        group_end = bond.hapticBondBegin().isAttachmentGroup() or bond.hapticBondEnd().isAttachmentGroup()
        expected = BOND_LENGTH * (GROUP_MULTIPLIER if group_end else 1.0)

        # Both ends inside one connected component: the edges between them already
        # settle the distance, and the layout leaves such a bond alone by design.
        same_component = component_of.get(begin_atoms[0]) == component_of.get(end_atoms[0])
        length = distance(begin, end)
        if not same_component and abs(length - expected) > TOLERANCE:
            problems.append("length %.3f, expected %.2f" % (length, expected))

        ends = ((bond.hapticBondBegin(), begin_atoms, end), (bond.hapticBondEnd(), end_atoms, begin))
        for endpoint, members, other in ends:
            if not endpoint.isAttachmentGroup() or same_component:
                continue
            if held.get(component_of.get(members[0]), 0) != 1:
                continue
            error = right_angle_error(members, positions, other)
            if error is not None and error > ANGLE_TOLERANCE:
                problems.append("bond meets a %d-atom group %.1f degrees off the perpendicular" % (len(members), error))

    if problems:
        failures += 1
        print("%s: %s" % (name, "; ".join(problems)))
    else:
        print("%s: %d haptic bond(s) OK" % (name, bonds))

print("files with a problem: %d" % failures)
