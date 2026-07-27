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
indigo.setOption("json-use-native-precision", True)

print("*** SMILES LAYOUT ***")

root = joinPathPy("reactions/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = ["932-agents"]

files.sort()
for filename in files:
    rea = indigo.loadReactionFromFile(os.path.join(root, filename + ".smi"))
    rea.layout()
    ket = rea.json()
    compare_diff(ref_path, filename + ".ket", ket)
