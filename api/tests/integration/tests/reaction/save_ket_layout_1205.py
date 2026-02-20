import difflib
import os
import sys


def find_diff(a, b):
    return "\n".join(difflib.unified_diff(a.splitlines(), b.splitlines()))


sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import (  # noqa: F401
    Bingo,
    BingoException,
    BingoObject,
    Indigo,
    IndigoException,
    IndigoInchi,
    IndigoObject,
    IndigoRenderer,
    dataPath,
    dir_exists,
    getIndigoExceptionText,
    getRefFilepath,
    getRefFilepath2,
    isIronPython,
    isJython,
    joinPathPy,
    makedirs,
    moleculeLayoutDiff,
    reactionLayoutDiff,
    relativePath,
    rmdir,
)

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)
indigo.setOption("json-use-native-precision", True)

r1 = indigo.loadReactionFromFile(
    joinPathPy("reactions/issue_1205.rxn", __file__)
)
ket_out = r1.json()

# with open(joinPathPy("reactions/issue_1205.ket", __file__), "w") as file:
#     file.write(ket_out)

with open(joinPathPy("reactions/issue_1205.ket", __file__), "r") as file:
    ket_ref = file.read()
diff = find_diff(ket_ref, ket_out)
if diff:
    print("Difference found:\n", diff)
else:
    print("Test passed")
