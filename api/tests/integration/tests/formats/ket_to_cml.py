import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** KET to CML ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = ["1878-ket-to-cml", "macro/sa-mono", "3086-star-cml"]

files.sort()
for filename in files:
    try:
        ket = indigo.loadMoleculeFromFile(
            os.path.join(root, filename + ".ket")
        )
    except:
        ket = indigo.loadQueryMoleculeFromFile(
            os.path.join(root, filename + ".ket")
        )
    cml = ket.cml()
    compare_diff(ref_path, filename + ".cml", cml)
