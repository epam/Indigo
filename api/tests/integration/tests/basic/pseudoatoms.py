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
    file_exists,
    file_size,
    getIndigoExceptionText,
    joinPathPy,
    makedirs,
    relativePath,
    rmdir,
    threading,
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
