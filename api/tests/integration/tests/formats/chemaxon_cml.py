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

print("*** Load ChemAxon CML format ***")
root = joinPathPy("molecules/chemaxon_cml", __file__)
files = os.listdir(root)
files.sort()
for filename in files:
    print(filename)
    try:
        mol = indigo.loadMoleculeFromFile(os.path.join(root, filename))
        print(mol.molfile())
        print(mol.cml())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
