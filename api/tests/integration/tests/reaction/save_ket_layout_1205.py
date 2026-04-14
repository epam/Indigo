import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
from env_indigo import *

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)
indigo.setOption("json-use-native-precision", True)

reactions_path = joinPathPy("reactions/", __file__)
filename = "issue_1205"

r1 = indigo.loadReactionFromFile(
    os.path.join(reactions_path, filename + ".rxn")
)
ket_out = r1.json()
compare_diff(reactions_path, filename + ".ket", ket_out)
