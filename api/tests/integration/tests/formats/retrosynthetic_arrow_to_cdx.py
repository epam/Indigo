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
indigo.setOption("ignore-stereochemistry-errors", True)
indigo.setOption("json-saving-pretty", True)
indigo.setOption("molfile-saving-skip-date", True)

ref_path = joinPathPy("ref/", __file__)
root_rea = joinPathPy("reactions/", __file__)

ket_filename = "ket_retrosynthetic_arrow"

reaction_types = [
    ("R1 => R2", "ket_retro_arrow"),
    ("R1 + R2 => R3", "ket_retro_arrow_sum_of_reactants"),
    ("=> R1", "ket_retro_arrow_to_product"),
    ("R1 =>", "ket_retro_arrow_from_reactant"),
    ("R1 => R2 -> R3", "ket_retro_arrow_and_simple_arrow"),
    ("R1 => R2 => R3", "ket_two_retro_arrows"),
    ("R1 -> R2 => R3 + R4", "ket_simple_arrow_retro_arrow_sum_of_products"),
]


for reaction, test_case_filename in reaction_types:
    print(
        "input file: "
        + test_case_filename
        + ".ket, reaction type: "
        + reaction
    )

    rc = indigo.loadReactionFromFile(
        os.path.join(root_rea, test_case_filename + ".ket")
    )

    filename = test_case_filename + ".b64cdx"

    cdx_b64 = rc.b64cdx()

    # with open(os.path.join(ref_path, filename), "w") as file:
    #     file.write(cdx_b64)

    with open(os.path.join(ref_path, filename), "r") as file:
        cdx_b64_ref = file.read()

    diff = find_diff(cdx_b64_ref, cdx_b64)
    if not diff:
        print(filename + ":SUCCEED")
    else:
        print(filename + ":FAILED")
        print(diff)
