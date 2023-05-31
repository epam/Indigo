import difflib
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)

from env_indigo import *  # noqa

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
