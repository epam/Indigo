import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)
indigo.setOption("molfile-saving-skip-date", True)

print("*** KET to MOL ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)
root_rea = joinPathPy("reactions/", __file__)

files = [
    "ket_with_mult_group",
]

files.sort()
for filename in files:
    mol = indigo.loadMoleculeFromFile(os.path.join(root, filename + ".ket"))
    ket = mol.molfile()
    compare_diff(ref_path, filename + ".mol", ket)
