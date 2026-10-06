"""
Issue #3927: a haptic bond holds atoms together the way an ordinary bond does,
so the component calls report a complex as one component, and a counter ion that
nothing bonds as another.
"""

import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
from env_indigo import Indigo, joinPathPy  # noqa

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)
# Left to itself the saver writes V2000, which has no form for a haptic bond.
indigo.setOption("molfile-saving-mode", "3000")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)


def print_components(molecule):
    print("components: %d" % molecule.countComponents())
    for component in molecule.iterateComponents():
        print(
            "  %d: atoms %s, %d bonds, %d haptic bonds"
            % (
                component.index(),
                [atom.index() for atom in component.iterateAtoms()],
                component.countBonds(),
                component.clone().countHapticBonds(),
            )
        )
    print(
        "  component of each atom: %s"
        % [atom.componentIndex() for atom in molecule.iterateAtoms()]
    )


print("*** a complex is one component ***")
for name in ["ferrocene-variant5", "ferrocene-variant6"]:
    print(name)
    ferrocene = indigo.loadMoleculeFromFile(os.path.join(root, name + ".mol"))
    print_components(ferrocene)
    compare_diff(ref_path, name + "_fragments.sdf", ferrocene.fragmentedSdf())

print("*** a counter ion is another ***")
salt = indigo.loadMoleculeFromFile(
    os.path.join(root, "ferrocene-variant5.mol")
)
salt.merge(indigo.loadMolecule("[Na+].[Cl-]"))
print_components(salt)
compare_diff(
    ref_path,
    "ferrocene-variant5_with_ions_fragments.sdf",
    salt.fragmentedSdf(),
)

print("*** a haptic bond joins components, and parts them when it goes ***")
half_sandwich = indigo.loadMolecule("C1=CC=CC1.[Fe]")
print_components(half_sandwich)
print("an attachment group over the ring, no bond to it yet")
group = half_sandwich.addAttachmentGroup([0, 1, 2, 3, 4])
print_components(half_sandwich)
print("a haptic bond from the group to the iron")
bond = half_sandwich.addHapticBond(group, half_sandwich.getAtom(5))
print_components(half_sandwich)
print("the haptic bond removed")
bond.remove()
print_components(half_sandwich)
