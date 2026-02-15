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


def foldThenSmiles(filename):
    print(relativePath(filename))
    mol = indigo.loadMoleculeFromFile(filename)
    mol.foldHydrogens()
    print(mol.smiles())
    mol.aromatize()
    print(mol.canonicalSmiles() + "\n")


foldThenSmiles(joinPathPy("molecules/li-h.mol", __file__))
foldThenSmiles(joinPathPy("molecules/pc-438107.mol", __file__))
foldThenSmiles(joinPathPy("molecules/pc-20749491.mol", __file__))
