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
indigo.setOption("molfile-saving-skip-date", True)

print("*** CDXML to mol ***")

root = joinPathPy("molecules/cdxml2", __file__)
for filename in sorted(os.listdir(root)):
    print(filename)
    try:
        mol = indigo.loadMoleculeFromFile(os.path.join(root, filename))
        print(mol.molfile())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
        print("*** Try as Query ***")
        mol = indigo.loadQueryMoleculeFromFile(os.path.join(root, filename))
        print(mol.molfile())
