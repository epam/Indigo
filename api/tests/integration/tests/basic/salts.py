"""
checkSalt() and stripSalt(): a salt is a component of a molecule that is a lone
ion or a small inorganic species. A component that haptic bonds hold together
is a coordination compound and never a salt (issue #3927).
"""

import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import Indigo  # noqa

indigo = Indigo()

RING_SIZE = 5


def print_check(smiles_list):
    for smiles in smiles_list:
        molecule = indigo.loadMolecule(smiles)
        print(
            "%s: %s" % (smiles, "salt" if molecule.checkSalt() else "no salt")
        )


def print_strip(smiles_list):
    for smiles in smiles_list:
        molecule = indigo.loadMolecule(smiles)
        print("%s -> '%s'" % (smiles, molecule.stripSalt().smiles()))


def bind_rings(smiles, ring_starts, metal):
    molecule = indigo.loadMolecule(smiles)
    for first in ring_starts:
        ring = list(range(first, first + RING_SIZE))
        group = molecule.addAttachmentGroup(ring)
        molecule.addHapticBond(group, molecule.getAtom(metal))
    return molecule


def describe(molecule):
    symbols = sorted([atom.symbol() for atom in molecule.iterateAtoms()])
    return "atoms %s, %d haptic bonds, %s" % (
        " ".join(symbols),
        molecule.countHapticBonds(),
        "salt" if molecule.checkSalt() else "no salt",
    )


print("*** a lone ion ***")
print_check(
    [
        "[Na+].C",
        "[Rb+].C",
        "[Ca+2].C",
        "[Zn+2].C",
        "[Al+3].C",
        "[Cr+3].C",
        "[Ru+4].C",
        "[Sn+4].C",
        "[Cl-].C",
        "[F-].C",
        "[S-2].C",
        "[Se-2].C",
    ]
)

print("*** a small inorganic molecule ***")
print_check(
    [
        "S=[Fe].C",
        "Cl[Ag]",
        "S=[Sn]=S.C",
        "O=[Mn]=O.C",
        "Cl[Fe](Cl)Cl.C",
        "OCl(=O)=O.C",
        "OS(=O)(=O)O.C",
        "OP(=O)(O)O.C",
    ]
)

print("*** an inorganic ion of several atoms ***")
print_check(
    [
        "[OH-].C",
        "[O-]Cl.C",
        "[O-]I=O.C",
        "[O-]N(=O).C",
        "[N+](=O)([O-])[O-].C",
        "O[Se](=O)[O-].C",
        "OP(=O)(O)[O-].C",
        "OS(=O)(=O)[O-].C",
    ]
)

print("*** several ions ***")
print_check(["[Na+].[Cl-].C", "[O-]S(=O)(=O)[O-].[K+].[K+].C"])

print("*** no salt: nothing but organic molecules ***")
print_check(["c1ccccc1", "C1=CC=C2C=CC=CC2=C1"])

print("*** no salt: the metal or the acid group is bonded ***")
print_check(
    [
        "CC[Pb](CC)(CC)CC",
        "C[Al](C)C",
        "C1=CC=C(C=C1)[N+](=O)[O-]",
        "C(C(=O)O)S(=O)(=O)O",
    ]
)

print("*** stripSalt ***")
print_strip(
    [
        "CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1",
        "CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1.[Cl-]",
        "CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1.O.O.O.O.O.O.O.O.O.O.[Cl-].[Cl-]",
        "CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1."
        "CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1."
        "[O-]S(=O)(=O)[O-]",
        "C(C(C(C(C(C(=O)O)O)O)O)O)O.C(C(C(C(C(C(=O)O)O)O)O)O)O.[Fe]",
        "[NH4+].[O-]P(=O)([O-])[O-].[Fe+2]",
    ]
)

print("*** the ion goes, not the atom whose place it has in the numbering ***")
print_strip(["C1.[Na+].C1"])

print("*** a copy is stripped unless the molecule itself is asked for ***")
chloride_salt = indigo.loadMolecule("CCCCCCCCCCCCCCCC[N+]1C=CC=CC=1.[Cl-]")
print("the copy: %s" % chloride_salt.stripSalt().smiles())
print("the molecule after it: %s" % chloride_salt.smiles())
chloride_salt.stripSalt(True)
print("stripped in place: %s" % chloride_salt.smiles())

print("*** a haptic complex is not a salt ***")
ferrocene = bind_rings("[cH-]1cccc1.[Fe+2].[cH-]1cccc1", [0, 6], 5)
print("ferrocene: %s" % describe(ferrocene))
print("stripped: %s" % describe(ferrocene.stripSalt()))

print("*** its counter ion is ***")
ferrocene.merge(indigo.loadMolecule("[Cl-]"))
print("ferrocene and a chloride: %s" % describe(ferrocene))
print("stripped: %s" % describe(ferrocene.stripSalt()))

print("*** a counter ion drawn between the atoms of the complex ***")
ferrocene = bind_rings("[cH-]1cccc1.[Cl-].[cH-]1cccc1.[Fe+2]", [0, 6], 11)
print("stripped: %s" % describe(ferrocene.stripSalt()))
ferrocene.stripSalt(True)
print("stripped in place: %s" % describe(ferrocene))

print("*** a metal with inorganic ligands, held by a haptic bond ***")
# Zeise's salt. By its ordinary bonds alone the platinum with three chlorides is
# a small inorganic ion; the haptic bond to the ethylene makes it a part of the
# complex. Nothing bonds the potassium.
zeise = indigo.loadMolecule("[K+].Cl[Pt-](Cl)Cl.C=C")
zeise.addHapticBond(zeise.addAttachmentGroup([5, 6]), zeise.getAtom(2))
print("Zeise's salt: %s" % describe(zeise))
print("stripped: %s" % describe(zeise.stripSalt()))
