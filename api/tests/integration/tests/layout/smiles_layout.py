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

print("*** SMILES LAYOUT ***")

root = joinPathPy("reactions/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = ["932-agents"]

files.sort()
for filename in files:
    rea = indigo.loadReactionFromFile(os.path.join(root, filename + ".smi"))
    rea.layout()
    ket = rea.json()

    # with open(os.path.join(ref_path, filename) + ".ket", "w") as file:
    #     file.write(ket)

    with open(os.path.join(ref_path, filename) + ".ket", "r") as file:
        ket_ref = file.read()
    diff = find_diff(ket_ref, ket)
    if not diff:
        print(filename + ".ket:SUCCEED")
    else:
        print(filename + ".ket:FAILED")
        print(diff)
