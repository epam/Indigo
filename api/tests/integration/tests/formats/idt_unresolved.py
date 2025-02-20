﻿import difflib
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import (  # noqa
    Indigo,
    IndigoException,
    getIndigoExceptionText,
    joinPathPy,
)

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** IDT unresolved to misc. unsupported formats ***")

lib = indigo.loadMonomerLibraryFromFile(
    os.path.join(joinPathPy("ref/", __file__), "monomer_library.ket")
)
mol = indigo.loadIdt("/i2AmPr/", lib)
try:
    mol.sequence(lib)
except IndigoException as e:
    print(getIndigoExceptionText(e))
try:
    mol.cdxml()
except IndigoException as e:
    print(getIndigoExceptionText(e))
try:
    mol.molfile()
except IndigoException as e:
    print(getIndigoExceptionText(e))
try:
    mol.smiles()
except IndigoException as e:
    print(getIndigoExceptionText(e))
try:
    mol.cml()
except IndigoException as e:
    print(getIndigoExceptionText(e))
