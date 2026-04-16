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
indigo.setOption("json-saving-pretty", True)
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** RXN to KET ***")

root = joinPathPy("reactions/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = [
    "2531-bad-layout",
    "2485-bad-layout",
    "rxn3000-with-macro",
    "2591-up-down",
    "quoted_subscript",
    "rxn_unused_rgroups",
]

files.sort()
for filename in files:
    mol = indigo.loadReactionFromFile(os.path.join(root, filename + ".rxn"))
    ket = mol.json()
    compare_diff(ref_path, filename + ".ket", ket)
