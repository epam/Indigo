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

files = [
    "1256-ketR20-from2815",
    "1256-salt-attached-rgroup",
    "1256-two-independent-rgroups",
    "1256-standard-plus-two-independent",
    "1256-query-independent-rgroup",
]

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

    sdf = ket.fragmentedSdf()
    compare_diff(ref_path, filename + ".sdf", sdf)

root = joinPathPy("reactions/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = [
    "1256-reaction-one-independent-rgroup",
    "1256-reaction-two-independent-rgroups",
]

files.sort()
for filename in files:
    try:
        ket = indigo.loadReactionFromFile(
            os.path.join(root, filename + ".ket")
        )
    except IndigoException:
        ket = indigo.loadQueryReactionFromFile(
            os.path.join(root, filename + ".ket")
        )

    sdf = ket.fragmentedSdf()
    compare_diff(ref_path, filename + ".sdf", sdf)
