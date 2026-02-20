import os
import sys

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

print("'Na' molecule loading..")
try:
    pml = indigo.loadMolecule("Na")
except IndigoException as e:
    print(getIndigoExceptionText(e))

print("Loop starting..")
fail_cnt = 0
for i in range(0, 25000):
    try:
        mol = indigo.createMolecule()
        count = mol.countComponents()
    except IndigoException as e:
        print(getIndigoExceptionText(e))
        fail_cnt += 1
print("fail_cnt=" + str(fail_cnt))
