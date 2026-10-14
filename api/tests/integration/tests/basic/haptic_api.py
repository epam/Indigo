"""
Issue #3842: expose attachment groups and haptic bonds (#3233) through the API.
Issue #3924: the charge and the radical of an attachment group.

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
from common.util import compare_diff  # noqa
from env_indigo import (  # noqa
    Indigo,
    IndigoException,
    getIndigoExceptionText,
    joinPathPy,
)

indigo = Indigo()

MOLFILE_SAVING_MODE = "molfile-saving-mode"
V3000 = "3000"

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


# A chlorine attached to any one of three benzene atoms: the Markush form of the
# same V3000 keys (#3731), whose star is not a phantom and stays in the structure.
VARIABLE_ATTACHMENT = """
  Indigo test

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 8 7 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0.0 0.0 0 0
M  V30 2 C 1.0 0.0 0 0
M  V30 3 C 1.5 0.866 0 0
M  V30 4 C 1.0 1.732 0 0
M  V30 5 C 0.0 1.732 0 0
M  V30 6 C -0.5 0.866 0 0
M  V30 7 Cl 2.5 0.866 0 0
M  V30 8 * 1.0 0.866 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 2 2 2 3
M  V30 3 1 3 4
M  V30 4 2 4 5
M  V30 5 1 5 6
M  V30 6 2 6 1
M  V30 7 1 7 8 ENDPTS=(3 2 3 4) ATTACH=ANY
M  V30 END BOND
M  V30 END CTAB
M  END
"""


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
indigo.setOption(MOLFILE_SAVING_MODE, V3000)
v3000 = mol.molfile()
print("V3000 carries ENDPTS: %s" % ("ENDPTS" in v3000))
print(
    "the group gets a star atom back: %s"
    % (indigo.loadMolecule(v3000).countAttachmentGroups() == 1)
)
indigo.setOption(MOLFILE_SAVING_MODE, "2000")
print("V2000 carries ENDPTS: %s" % ("ENDPTS" in mol.molfile()))
indigo.setOption(MOLFILE_SAVING_MODE, "auto")

print()
print(
    "*** V3000 absorbs the star of a haptic record, not a variable one's ***"
)
from_v3000 = indigo.loadMolecule(v3000)
print(
    "haptic star in the structure: %s"
    % any(a.symbol() == "*" for a in from_v3000.iterateAtoms())
)
print(
    "haptic group anchor: %s"
    % (from_v3000.getAttachmentGroup(0).getAttachmentGroupAnchor() or "none")
)
variable = indigo.loadMolecule(VARIABLE_ATTACHMENT)
anchor = variable.getAttachmentGroup(0).getAttachmentGroupAnchor()
print(
    "variable attachment anchor: %s"
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


# Issue #3924. What a saver writes is compared with a reference file; the .out
# keeps the values read back through the API and the error texts.
molecules_path = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/haptic_api/", __file__)
# One group, ethylene on platinum, with no charge of its own; the atoms K+ and
# Pt- carry charges the group must leave alone.
ZEISE_SALT = "zeise-salt-hydrate"
# Two groups, whose stars bring a charge of -1 each.
FERROCENE = "ferrocene-variant5"
indigo.setOption("json-saving-pretty", True)
indigo.setOption("json-use-native-precision", True)
indigo.setOption("json-set-native-precision", 3)
indigo.setOption("molfile-saving-skip-date", True)
# Left to itself the saver writes V2000, which has no form for a haptic bond.
indigo.setOption(MOLFILE_SAVING_MODE, V3000)


def fixture_path(name):
    return os.path.join(molecules_path, name + ".mol")


def group_values(group):
    return "charge=%s radical=%s" % (group.charge(), group.radical())


def atom_charges(mol):
    return [atom.charge() for atom in mol.iterateAtoms()]


print()
print("*** a group has a charge and a radical of its own ***")
zeise = indigo.loadMoleculeFromFile(fixture_path(ZEISE_SALT))
compare_diff(ref_path, ZEISE_SALT + ".ket", zeise.json())
ethylene_group = zeise.getAttachmentGroup(0)
print("as loaded: %s" % group_values(ethylene_group))
charges_before = atom_charges(zeise)
ethylene_group.setCharge(-1)
ethylene_group.setRadical(Indigo.DOUBLET)
print("after setting: %s" % group_values(ethylene_group))
print("atom charges unchanged: %s" % (atom_charges(zeise) == charges_before))
for haptic_bond in zeise.iterateHapticBonds():
    for bond_end in (
        haptic_bond.hapticBondBegin(),
        haptic_bond.hapticBondEnd(),
    ):
        if bond_end.isAttachmentGroup() and bond_end.index() == 0:
            print(
                "the same group as the end of its bond: %s"
                % group_values(bond_end)
            )
charged_ket = zeise.json()
charged_molfile = zeise.molfile()
compare_diff(ref_path, ZEISE_SALT + "-charged.ket", charged_ket)
compare_diff(ref_path, ZEISE_SALT + "-charged.mol", charged_molfile)
print(
    "read back from KET: %s"
    % group_values(indigo.loadMolecule(charged_ket).getAttachmentGroup(0))
)
print(
    "read back from V3000: %s"
    % group_values(indigo.loadMolecule(charged_molfile).getAttachmentGroup(0))
)
expect_error("unknown radical", lambda: ethylene_group.setRadical(5))
expect_error("same on an atom", lambda: zeise.getAtom(0).setRadical(5))

print()
print("*** every radical value, and back to none ***")
for radical_name, radical_value in (
    ("singlet", Indigo.SINGLET),
    ("doublet", Indigo.DOUBLET),
    ("triplet", Indigo.TRIPLET),
    ("none", 0),
):
    ethylene_group.setRadical(radical_value)
    print(
        "%s: set %s, read %s"
        % (radical_name, radical_value, ethylene_group.radical())
    )
ethylene_group.setCharge(0)
print("charge set back: %s" % ethylene_group.charge())
# Cleared, the group is saved exactly as the file was loaded.
compare_diff(ref_path, ZEISE_SALT + ".ket", zeise.json())

print()
print("*** two groups keep their own values ***")
two_rings = indigo.loadMoleculeFromFile(fixture_path(FERROCENE))
first_ring = two_rings.getAttachmentGroup(0)
second_ring = two_rings.getAttachmentGroup(1)
first_ring.setCharge(1)
second_ring.setCharge(2)
second_ring.setRadical(Indigo.TRIPLET)
for group in two_rings.iterateAttachmentGroups():
    print("group %d: %s" % (group.index(), group_values(group)))
first_ring.remove()
expect_error("removed: charge", lambda: first_ring.charge())
expect_error("removed: setCharge", lambda: first_ring.setCharge(1))
expect_error("removed: radical", lambda: first_ring.radical())
expect_error(
    "removed: setRadical", lambda: first_ring.setRadical(Indigo.DOUBLET)
)
print("the other group is as it was: %s" % group_values(second_ring))

print()
print("*** a view taken from the molecule follows the group ***")
viewed = indigo.loadMoleculeFromFile(fixture_path(ZEISE_SALT))
view = viewed.getSubmolecule([atom.index() for atom in viewed.iterateAtoms()])


def view_groups():
    return ", ".join(
        group_values(group) for group in view.iterateAttachmentGroups()
    )


print("before: %s" % view_groups())
viewed.getAttachmentGroup(0).setCharge(-1)
print("after setCharge: %s" % view_groups())
viewed.getAttachmentGroup(0).setRadical(Indigo.DOUBLET)
print("after setRadical: %s" % view_groups())

print()
print("*** a group of a query molecule takes them as well ***")
query = indigo.loadQueryMoleculeFromFile(fixture_path(ZEISE_SALT))
query_group = query.getAttachmentGroup(0)
query_group.setCharge(-1)
query_group.setRadical(Indigo.TRIPLET)
print(group_values(query_group))

print()
print("*** no other atom call takes a group, and no other object these ***")
expect_error("resetCharge", lambda: ethylene_group.resetCharge())
expect_error("resetRadical", lambda: ethylene_group.resetRadical())
expect_error("radicalElectrons", lambda: ethylene_group.radicalElectrons())
expect_error("charge of a bond", lambda: zeise.getBond(0).charge())
expect_error("of a haptic bond", lambda: zeise.getHapticBond(0).setCharge(1))

print()
print("*** V3000 keeps the charge of a haptic group only ***")
markush = indigo.loadMolecule(VARIABLE_ATTACHMENT)
markush.getAttachmentGroup(0).setCharge(-1)
expect_error("variable attachment", lambda: markush.molfile())
compare_diff(ref_path, "variable-attachment-charged.ket", markush.json())
