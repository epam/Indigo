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

# indigo::SmilesLoader::Error
try:
    m = indigo.loadMolecule("CX")
except IndigoException as e:
    print(getIndigoExceptionText(e))

# IndigoError
try:
    r = indigo.loadReaction("C1=CC=CC=C1>>C1=CC=CC=C1")
    for atom in r.iterateAtoms():
        print(atom)
except IndigoException as e:
    print(getIndigoExceptionText(e))
