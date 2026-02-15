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
print(
    indigo.loadMoleculeFromFile(
        joinPathPy("molecules/13rsites.mol", __file__)
    ).smiles()
)
print(
    indigo.loadMoleculeFromFile(
        joinPathPy("molecules/1e-0.mol", __file__)
    ).smiles()
)

indigo.setOption("ignore-stereochemistry-errors", True)
print(
    indigo.loadMoleculeFromFile(
        joinPathPy("molecules/atropisomer.mol", __file__)
    ).smiles()
)

print(
    indigo.loadMoleculeFromFile(
        joinPathPy("molecules/macro/sa-mono.mol", __file__)
    ).smiles()
)

print(
    indigo.loadMoleculeFromFile(
        joinPathPy("molecules/3088-star-smarts.mol", __file__)
    ).smarts()
)
