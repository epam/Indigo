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

from rendering import *

if __name__ == "__main__":
    indigo = Indigo()
    indigo.setOption("molfile-saving-skip-date", True)
    rfile = open(joinPathPy("molecules/multistep.ket", __file__))
    reaction = rfile.read()
    r = indigo.loadReaction(reaction)
    print(r.smiles())
    print(r.rxnfile())
