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
for molecule in indigo.iterateSmilesFile(
    dataPath("molecules/basic/sample_100000.smi")
):
    hash_ = molecule.hash()
    if hash_ in hashes:
        if not indigo.exactMatch(
            molecule, indigo.loadMolecule(hashes[hash_]), "NONE"
        ):
            print(
                "Hash for molecule {} collides with molecule {}".format(
                    molecule.rawData(), hashes[hash_]
                )
            )
    else:
        hashes[hash_] = molecule.rawData()
