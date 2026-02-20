import errno
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
mol = indigo.loadMolecule(
    indigo.loadMolecule("c1cccc(-c2ccc(Nc3cccc4c(=O)[nH]ccc34)nc2)c1").json()
)
print(mol.smiles())
mol.dearomatize()
s = ""
for a in mol.iterateAtoms():
    s = s + " " + str(a.countImplicitHydrogens())
print(s)
