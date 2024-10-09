import difflib
import os
import sys


def find_diff(a, b):
    return "\n".join(difflib.unified_diff(a.splitlines(), b.splitlines()))


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

root = joinPathPy("molecules/macro/", __file__)

mol = indigo.loadMoleculeFromFile(os.path.join(root, "ambiguous.ket"))
print("Test mol v2000")
try:
    indigo.setOption("molfile-saving-mode", "2000")
    print(mol.molfile())
except IndigoException as e:
    print(getIndigoExceptionText(e))
print("Test mol v3000")
try:
    indigo.setOption("molfile-saving-mode", "3000")
    print(mol.molfile())
except IndigoException as e:
    print(getIndigoExceptionText(e))
print("Test cdxml")
try:
    print(mol.cdxml())
except IndigoException as e:
    print(getIndigoExceptionText(e))
print("Test cml")
try:
    print(mol.cml())
except IndigoException as e:
    print(getIndigoExceptionText(e))
print("Test smiles")
try:
    print(mol.smiles())
except IndigoException as e:
    print(getIndigoExceptionText(e))
