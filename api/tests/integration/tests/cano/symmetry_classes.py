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

idx = 1
for idx, m in enumerate(
    indigo.iterateSDFile(joinPathPy("molecules/symmetry.sdf", __file__))
):
    print("Item #%d" % (idx))
    classes = m.symmetryClasses()
    for a in m.iterateAtoms():
        print("  %d: %d" % (a.index(), classes[a.index()]))
