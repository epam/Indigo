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

hashes = {}
with open(dataPath("reactions/basic/40000.rsmi")) as f:
    for smiles in f:
        reaction = indigo.loadReaction(smiles)
        hash_ = reaction.hash()
        if hash_ in hashes:
            if not indigo.exactMatch(
                reaction, indigo.loadReaction(hashes[hash_]), "NONE"
            ):
                print(
                    "Hash for reaction {} collides with reactions {}".format(
                        smiles, hashes[hash_]
                    )
                )
        else:
            hashes[hash_] = smiles
