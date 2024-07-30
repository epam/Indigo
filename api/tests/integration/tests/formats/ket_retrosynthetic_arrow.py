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
indigo.setOption("ignore-stereochemistry-errors", True)
indigo.setOption("json-saving-pretty", True)
indigo.setOption("molfile-saving-skip-date", True)

ref_path = joinPathPy("ref/", __file__)
root_rea = joinPathPy("reactions/", __file__)

ket_filename = "ket_retrosynthetic_arrow"


def getV2000(reaction):
    indigo.setOption("molfile-saving-mode", "2000")
    return reaction.rxnfile()


def getV3000(reaction):
    indigo.setOption("molfile-saving-mode", "3000")
    return reaction.rxnfile()


def getSmiles(reaction):
    return reaction.smiles()


def getSmarts(reaction):
    return reaction.smarts()


def getCML(reaction):
    return reaction.cml()


reaction_types = [
    ("R1 => R2", "ket_retro_arrow"),
    ("R1 + R2 => R3", "ket_retro_arrow_sum_of_reactants"),
    ("=> R1", "ket_retro_arrow_to_product"),
    ("R1 =>", "ket_retro_arrow_from_reactant"),
    ("R1 => R2 -> R3", "ket_retro_arrow_and_simple_arrow"),
    ("R1 => R2 => R3", "ket_two_retro_arrows"),
    ("R1 -> R2 => R3 + R4", "ket_simple_arrow_retro_arrow_sum_of_products"),
]


output_formats = [
    ("RXN 2000", "_2000.rxn", getV2000),
    ("RXN 3000", "_3000.rxn", getV3000),
    ("SMILES", ".smi", getSmiles),
    ("SMARTS", ".smarts", getSmarts),
    ("CML", ".cml", getCML),
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
    for format_name, format_postfix, get_format_func in output_formats:
        filename = test_case_filename + format_postfix

        parsed_format = get_format_func(rc)

        print("output format: " + format_name)

        with open(os.path.join(ref_path, filename), "r") as file:
            format_ref = file.read()
        diff = find_diff(format_ref, parsed_format)
        if not diff:
            print(filename + ":SUCCEED")
        else:
            print(filename + ":FAILED")
            print(diff)
