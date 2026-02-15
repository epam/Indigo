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

i = Indigo()
r1 = i.loadReactionFromFile(joinPathPy("reactions/q_43.rxn", __file__))
r2 = i.loadReactionFromFile(joinPathPy("reactions/q_43.rxn", __file__))
print(i.exactMatch(r1, r2) is not None)
print(i.exactMatch(r1, r2, "AAM") is not None)
print(i.exactMatch(r1, r2, "STE MAS") is not None)
print(i.exactMatch(r1, r2, "NONE") is not None)
