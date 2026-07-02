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
indigo.setOption("molfile-saving-skip-date", True)
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** KET to SDF ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = ["1256-ketR20-from2815"]

files.sort()
for filename in files:
    try:
        ket = indigo.loadMoleculeFromFile(
            os.path.join(root, filename + ".ket")
        )
    except IndigoException:
        ket = indigo.loadQueryMoleculeFromFile(
            os.path.join(root, filename + ".ket")
        )

    sdf = ket.getFragmentSdf()
    compare_diff(ref_path, filename + ".sdf", sdf)
