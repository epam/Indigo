import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
from env_indigo import Indigo, joinPathPy  # noqa

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)
indigo.setOption("ignore-stereochemistry-errors", True)
indigo.setOption("molfile-saving-skip-date", True)

print("*** KET to RXN ***")

root = joinPathPy("reactions/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = [
    "merge_test1",
    "merge_test1a",
    "merge_test2",
    "merge_test2a",
    "merge_test3",
    "merge_test3a",
    "merge_test4",
    "merge_test5",
    "merge_test6",
    "merge_test7",
    "merge_test8",
    "merge_test9",
    "rxn_unused_rgroups",
]

files.sort()
for filename in files:
    rea = indigo.loadReactionFromFile(os.path.join(root, filename + ".ket"))
    rxn = rea.rxnfile()
    compare_diff(ref_path, filename + ".rxn", rxn)
