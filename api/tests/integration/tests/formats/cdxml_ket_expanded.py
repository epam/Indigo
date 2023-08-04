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
mol = indigo.loadMoleculeFromFile(
    joinPathPy("ref/cdxml_ket_expanded.cdxml", __file__)
)

print(mol.json())
