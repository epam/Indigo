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

m = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/aniline_pol_psd.mol", __file__)
)
print("aniline_pol_psd.mol")
for a in m.iteratePseudoatoms():
    print(a.symbol())
print(m.molfile())
m = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/aniline_pol_psd_alias.mol", __file__)
)
print("aniline_pol_psd_alias.mol")
for a in m.iteratePseudoatoms():
    print(a.symbol())
print(m.molfile())
