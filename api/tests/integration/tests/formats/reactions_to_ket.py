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
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)
indigo.setOption("ignore-stereochemistry-errors", True)

print("*** SMARTS to KET ***")

ref_path = joinPathPy("ref/", __file__)

tests = [
    (
        "[#6]1(-[#6])-[#6](-[#8])=[#6]-[#6](-[#16])=[#6](-[#7])-[#6]=1>>[#6]1(-Br)-[#6](-[#6])=[#6]-[#6](-I)=[#6](-[#8])-[#6]=1",
        "2662-arrow-size",
    ),
    ("[#15]1-[#6]=[#6]-[#6]=[#6]-1>Br.Br>[#8]", "2664-catalyst-margin"),
    ("C1=OC=CC=C1>C1N=CC=C1>", "2665-incomplete-reaction"),
]

for test in tests:
    try:
        reaction = indigo.loadReaction(test[0])
    except:
        try:
            reaction = indigo.loadQueryReaction(test[0])
        except:
            print("bad reaction data")

    # with open(os.path.join(ref_path, test[1]) + ".ket", "w") as file:
    #     file.write(reaction.json())

    with open(os.path.join(ref_path, test[1]) + ".ket", "r") as file:
        ket_ref = file.read()

    ket = reaction.json()
    diff = find_diff(ket_ref, ket)
    if not diff:
        print(test[1] + ".ket:SUCCEED")
    else:
        print(test[1] + ".ket:FAILED")
        print(diff)
