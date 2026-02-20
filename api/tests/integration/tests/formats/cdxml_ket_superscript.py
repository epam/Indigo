import difflib
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

indigo.setOption("json-saving-pretty", True)
print("*** CDXML to KET ***")
ref_path = joinPathPy("ref/", __file__)
mol_superscript = indigo.loadMoleculeFromFile(
    joinPathPy("cdxml/x_supescript2.cdxml", __file__)
)

print(mol_superscript.json())

mol_subscript = indigo.loadMoleculeFromFile(
    joinPathPy("cdxml/x_subscript2.cdxml", __file__)
)

print(mol_subscript.json())

mol_3d_structure = indigo.loadMoleculeFromFile(
    joinPathPy("cdxml/1944-3D_Structure.cdxml", __file__)
)

print(mol_3d_structure.json())
