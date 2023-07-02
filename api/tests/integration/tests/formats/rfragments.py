import difflib
import os
import sys
from deepdiff import DeepDiff


def find_diff(a, b):
    return "\n".join(difflib.unified_diff(a.splitlines(), b.splitlines()))


sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)
root_mol = joinPathPy("molecules/", __file__)
filename = "rfragments.ket"
with open(os.path.join(root_mol, filename), "r") as file:
    ket_ref = file.read()
    try:
        mol = indigo.loadMolecule(ket_ref)
    except IndigoException as e:
        print(getIndigoExceptionText(e))
    print(mol.json())
