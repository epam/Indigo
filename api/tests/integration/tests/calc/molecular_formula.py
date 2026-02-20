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

print("3012_iupac_molecular_formula")
indigo = Indigo()
m = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/3012_iupac_molecular_formula.mol", __file__)
)
print(m.grossFormula())
print(m.molecularFormula())
indigo.setOption("gross-formula-add-isotopes", True)
print(m.molecularFormula())

indigo.setOption("gross-formula-add-isotopes", False)
r = indigo.loadReactionFromFile(
    joinPathPy("reactions/3012_iupac_molecular_formula.rxn", __file__)
)
print(r.grossFormula())
print(r.molecularFormula())
indigo.setOption("gross-formula-add-isotopes", True)
print(r.molecularFormula())
