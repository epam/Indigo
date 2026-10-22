import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
from env_indigo import *  # noqa

SUBFOLDER = "sgroup"
SAVING_MODE = "molfile-saving-mode"
BOTH_VERSIONS = ("2000", "3000")

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)
indigo.setOption("json-saving-pretty", True)
indigo.setOption("json-use-native-precision", True)
indigo.setOption("json-set-native-precision", 3)

root = joinPathPy("molecules/" + SUBFOLDER, __file__)
ref_path = joinPathPy("ref/" + SUBFOLDER, __file__)


def load(name):
    return indigo.loadMoleculeFromFile(os.path.join(root, name))


print("*** merge() into a molecule that has an S-group ***")
# the copies of the nested S-groups of the formulation keep their parents
merged = load("generic-on-two-nitrogens-v2000.mol")
merged.merge(load("formulation-v2000.mol"))
for mode in BOTH_VERSIONS:
    indigo.setOption(SAVING_MODE, mode)
    compare_diff(
        ref_path, "formulation-v2000-merged-%s.mol" % mode, merged.molfile()
    )

print("*** A parent and its child on different fragments ***")
filename = "parent-on-another-fragment-v3000"
mol = load(filename + ".mol")
compare_diff(ref_path, filename + ".ket", mol.json())
# each component keeps the S-groups on its own atoms
indigo.setOption(SAVING_MODE, "3000")
for number, component in enumerate(mol.iterateComponents()):
    compare_diff(
        ref_path,
        "%s-component-%d.mol" % (filename, number),
        component.clone().molfile(),
    )
