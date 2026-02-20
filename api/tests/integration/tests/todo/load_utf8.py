# coding=utf-8
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
indigo.setOption("molfile-saving-skip-date", "1")

print("****** Load molfile with UTF-8 characters in Data S-group ********")
m = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/sgroups_utf8.mol", __file__)
)
indigo.setOption("molfile-saving-mode", "2000")
res = m.molfile()
m = indigo.loadMolecule(res)

print(m.molfile())
print(res)
print(m.cml())
