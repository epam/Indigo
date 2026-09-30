"""
Issue #3927: a haptic bond holds atoms together the way an ordinary bond does,
so a complex is one component - the same one KET writes as one molecule node.

For every haptic structure the suite has, the component sizes the API reports
are printed beside the sizes of the KET molecule nodes; the two must agree.
"""

# The Java job runs this suite under Jython 2.7, where a bare print() would print
# an empty tuple instead of an empty line.
from __future__ import print_function

import json
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import Indigo, joinPathPy  # noqa

indigo = Indigo()


def component_sizes(molecule):
    return sorted(
        component.countAtoms() for component in molecule.iterateComponents()
    )


def ket_node_sizes(molecule):
    document = json.loads(molecule.json())
    return sorted(
        len(document[node["$ref"]]["atoms"])
        for node in document["root"]["nodes"]
    )


def report(name, molecule):
    sizes = component_sizes(molecule)
    same = sizes == ket_node_sizes(molecule)
    verdict = "as in KET" if same else "NOT AS IN KET"
    print("%-52s %s %s" % (name, sizes, verdict))


print("*** Components of haptic structures ***")

layout_root = joinPathPy("../layout/molecules/", __file__)
for filename in sorted(os.listdir(layout_root)):
    path = os.path.join(layout_root, filename)
    if not filename.endswith(".ket"):
        continue
    with open(path) as ket:
        if '"haptic"' not in ket.read():
            continue
    report(filename[: -len(".ket")], indigo.loadMoleculeFromFile(path))

for filename in ["ferrocene-variant5.mol", "ferrocene-variant6.mol"]:
    path = joinPathPy("molecules/" + filename, __file__)
    report(filename[: -len(".mol")], indigo.loadMoleculeFromFile(path))

print("*** A complex and its counter ions ***")

ferrocene = joinPathPy("molecules/ferrocene-variant5.mol", __file__)
salt = indigo.loadMoleculeFromFile(ferrocene)
salt.merge(indigo.loadMolecule("[Na+].[Cl-]"))
report("ferrocene with NaCl", salt)
for component in salt.iterateComponents():
    clone = component.clone()
    print(
        "component %d: %d atoms, %d haptic bonds, %d attachment groups"
        % (
            component.index(),
            clone.countAtoms(),
            clone.countHapticBonds(),
            clone.countAttachmentGroups(),
        )
    )
print("fragmentedSdf records: %d" % salt.fragmentedSdf().count("$$$$"))
