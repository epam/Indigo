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
indigo.setOption("molfile-saving-skip-date", True)

print("*** CDXML to mol ***")

root = joinPathPy("molecules/cdxml2", __file__)
files = os.listdir(root)
files.sort()
for filename in files:
    print(filename)
    try:
        mol = indigo.loadMoleculeFromFile(os.path.join(root, filename))
        print(mol.molfile())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
        print("*** Try as Query ***")
        try:
            mol = indigo.loadQueryMoleculeFromFile(
                os.path.join(root, filename)
            )
            print(mol.molfile())
        except IndigoException as e:
            print(getIndigoExceptionText(e))
            print("*** Try as Reaction ***")
            try:
                reac = indigo.loadReactionFromFile(
                    os.path.join(root, filename)
                )
                print(reac.rxnfile())
            except IndigoException as e:
                print(getIndigoExceptionText(e))
