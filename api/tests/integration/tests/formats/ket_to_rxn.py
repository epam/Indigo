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
]

files.sort()
for filename in files:
    rea = indigo.loadReactionFromFile(os.path.join(root, filename + ".ket"))

    # with open(os.path.join(ref_path, filename) + ".rxn", "w") as file:
    #     file.write(rea.rxnfile())
    with open(os.path.join(ref_path, filename) + ".rxn", "r") as file:
        rxn_ref = file.read()
    rxn = rea.rxnfile()
    diff = find_diff(rxn_ref, rxn)
    if not diff:
        print(filename + ".rxn:SUCCEED")
    else:
        print(filename + ".rxn:FAILED")
        print(diff)
