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


def loadSMILES(smiles):
    try:
        mol = indigo.loadMolecule(smiles)
        print(smiles + " : OK")
    except IndigoException as e:
        print(smiles + " " + getIndigoExceptionText(e))


loadSMILES("O[C@H](N)C |&1:0|")
loadSMILES("O[C@H](N)C |o1:0|")
loadSMILES("O[C@H](N)C |a:3|")
