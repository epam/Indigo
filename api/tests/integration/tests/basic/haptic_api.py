"""
Issue #3842: expose attachment groups and haptic bonds (#3233) through the API.

An attachment group is a set of atoms acting as one end of a haptic bond. Neither
the group nor the bond is a part of the molecular graph, so this test also pins
what must NOT change: the bond count, the valences and the implicit hydrogens.
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
from env_indigo import Indigo, IndigoException, getIndigoExceptionText  # noqa

indigo = Indigo()

KET_RING_AND_METAL = """{
    "root": {
        "nodes": [{"$ref": "mol0"}, {"$ref": "mol1"}],
        "connections": [
            {"type": "haptic",
             "endpoint1": {"atomId": "0", "moleculeId": "mol0"},
             "endpoint2": {"attachmentGroupId": "0", "moleculeId": "mol1"}}
        ]
    },
    "mol0": {"type": "molecule", "atoms": [{"label": "Fe", "location": [0, 0, 0]}]},
    "mol1": {
        "type": "molecule",
        "atoms": [
            {"label": "C", "location": [1, 0, 0]}, {"label": "C", "location": [2, 0, 0]},
            {"label": "C", "location": [3, 0, 0]}, {"label": "C", "location": [4, 0, 0]},
            {"label": "C", "location": [5, 0, 0]}
        ],
        "bonds": [
            {"type": 1, "atoms": [0, 1]}, {"type": 1, "atoms": [1, 2]},
            {"type": 1, "atoms": [2, 3]}, {"type": 1, "atoms": [3, 4]},
            {"type": 1, "atoms": [4, 0]}
        ],
        "attachmentGroups": [{"id": "0", "atoms": [0, 1, 2, 3, 4]}]
    }
}"""


def describe(mol, title):
    print(
        "%s: groups=%d haptic bonds=%d"
        % (title, mol.countAttachmentGroups(), mol.countHapticBonds())
    )
    for group in mol.iterateAttachmentGroups():
        print(
            "  group %d: atoms %s"
            % (group.index(), [a.index() for a in group.iterateAtoms()])
        )
    for bond in mol.iterateHapticBonds():
        print(
            "  bond %d: type=%s begin=%s end=%s"
            % (
                bond.index(),
                bond.hapticBondType(),
                endpoint_str(bond.hapticBondBegin()),
                endpoint_str(bond.hapticBondEnd()),
            )
        )


def endpoint_str(end):
    """An end is either an atom or an attachment group, and only the group knows
    itself as one."""
    if end.isAttachmentGroup():
        return "group %d %s" % (
            end.index(),
            [a.index() for a in end.iterateAtoms()],
        )
    return "atom %d (%s)" % (end.index(), end.symbol())


def expect_error(title, action):
    try:
        action()
        print("%-24s NO ERROR - unexpected" % title)
    except IndigoException as e:
        print("%-24s %s" % (title, getIndigoExceptionText(e)))


print("*** building a haptic complex through the API ***")
mol = indigo.loadMolecule("C1=CC=CC1.[Fe]")
ring = [a.index() for a in mol.iterateAtoms() if a.symbol() == "C"]
metal = [a for a in mol.iterateAtoms() if a.symbol() == "Fe"][0]

bonds_before = mol.countBonds()
implicit_before = [a.countImplicitHydrogens() for a in mol.iterateAtoms()]
valence_before = [a.valence() for a in mol.iterateAtoms()]

group = mol.addAttachmentGroup(ring)
haptic = mol.addHapticBond(group, metal)
describe(mol, "after building")
print("group countAtoms=%d" % group.countAtoms())

print()
print("*** the graph is untouched (isolation invariant of #3837) ***")
print("bond count unchanged: %s" % (bonds_before == mol.countBonds()))
print(
    "implicit hydrogens unchanged: %s"
    % (
        implicit_before
        == [a.countImplicitHydrogens() for a in mol.iterateAtoms()]
    )
)
print(
    "valences unchanged: %s"
    % (valence_before == [a.valence() for a in mol.iterateAtoms()])
)
print("bonds of the molecule: %d" % len([b for b in mol.iterateBonds()]))

print()
print("*** what the API builds survives a KET round-trip ***")
reloaded = indigo.loadMolecule(mol.json())
describe(reloaded, "reloaded")

print()
print("*** what KET brings in is visible through the API ***")
from_ket = indigo.loadMolecule(KET_RING_AND_METAL)
describe(from_ket, "from KET")
print(
    "getAttachmentGroup(0) countAtoms=%d"
    % from_ket.getAttachmentGroup(0).countAtoms()
)
print("getHapticBond(0) type=%s" % from_ket.getHapticBond(0).hapticBondType())

print()
print(
    "*** clone keeps them, a submolecule applies the all-or-nothing rule ***"
)
describe(mol.clone(), "clone")
describe(
    mol.getSubmolecule(ring).clone(), "whole ring without the metal"
)  # the bond loses its partner
describe(
    mol.getSubmolecule(ring[:3]).clone(), "part of the ring"
)  # the group loses members

print()
print(
    "*** formats that cannot express a haptic bond omit it, without failing ***"
)
for fmt, save in (
    ("smiles", mol.smiles),
    ("canonical smiles", mol.canonicalSmiles),
    ("cml", mol.cml),
):
    try:
        save()
        print("%-18s saved" % fmt)
    except IndigoException as e:
        print("%-18s %s" % (fmt, getIndigoExceptionText(e)))

print()
print("*** V3000 writes the bond out, V2000 omits it (#3840) ***")
indigo.setOption("molfile-saving-mode", "3000")
v3000 = mol.molfile()
print("V3000 carries ENDPTS: %s" % ("ENDPTS" in v3000))
print(
    "the group gets a star atom back: %s"
    % (indigo.loadMolecule(v3000).countAttachmentGroups() == 1)
)
indigo.setOption("molfile-saving-mode", "2000")
print("V2000 carries ENDPTS: %s" % ("ENDPTS" in mol.molfile()))
indigo.setOption("molfile-saving-mode", "auto")

print()
print(
    "*** a group loaded from V3000 knows its anchor atom, a KET one does not ***"
)
from_v3000 = indigo.loadMolecule(v3000)
anchor = from_v3000.getAttachmentGroup(0).getAttachmentGroupAnchor()
print(
    "V3000 anchor: %s"
    % (
        "atom %d (%s)" % (anchor.index(), anchor.symbol())
        if anchor is not None
        else "none"
    )
)
print(
    "KET anchor: %s"
    % (from_ket.getAttachmentGroup(0).getAttachmentGroupAnchor() or "none")
)

print()
print("*** membership can be replaced ***")
group.setAttachmentGroupAtoms(ring[:3])
print(
    "after setAttachmentGroupAtoms: atoms %s"
    % [a.index() for a in group.iterateAtoms()]
)
group.setAttachmentGroupAtoms(ring)

print()
print("*** errors ***")
metal_group = mol.addAttachmentGroup([metal.index()])
expect_error("group to group", lambda: mol.addHapticBond(group, metal_group))
expect_error(
    "bond to own member",
    lambda: mol.addHapticBond(group, mol.getAtom(ring[0])),
)
expect_error(
    "partner joins group",
    lambda: group.setAttachmentGroupAtoms(ring + [metal.index()]),
)
expect_error("atom out of range", lambda: mol.addAttachmentGroup([99]))
expect_error("no such group", lambda: mol.getAttachmentGroup(99))
expect_error("no such haptic bond", lambda: mol.getHapticBond(99))
expect_error("empty group", lambda: mol.addAttachmentGroup([]))

print()
print("*** removing a group takes its bonds with it ***")
group.remove()
print(
    "groups=%d haptic bonds=%d"
    % (mol.countAttachmentGroups(), mol.countHapticBonds())
)
expect_error("removed group is gone", lambda: group.countAtoms())

print()
print("*** removing a haptic bond leaves the group alone ***")
mol2 = indigo.loadMolecule(KET_RING_AND_METAL)
mol2.getHapticBond(0).remove()
print(
    "groups=%d haptic bonds=%d"
    % (mol2.countAttachmentGroups(), mol2.countHapticBonds())
)
