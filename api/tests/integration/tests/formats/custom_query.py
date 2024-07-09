#!/bin/env python3
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

indigo = Indigo()

ref_path = joinPathPy("ref/", __file__)


def test_smarts_to_ket(smarts_in, expected_str):
    mol = indigo.loadSmarts(smarts_in)
    json_out = mol.json()
    if expected_str in json_out:
        print("%s OK. Expected string found." % smarts_in)
    else:
        print(
            "FAILED: Expected string\n%s\n not found in \n%s\n"
            % (expected_str, json_out)
        )


def test_ket_to_smarts(filename, expected_str):
    mol = indigo.loadQueryMoleculeFromFile(os.path.join(ref_path, filename))
    smarts = mol.smarts()
    if smarts == expected_str:
        print(
            "%s OK. Smarts equals expected string '%s'"
            % (filename, expected_str)
        )
    else:
        print(
            "%s FAILED. Expected '%s', generated smarts '%s'"
            % (filename, expected_str, smarts)
        )


print("**** #1310 error at opening SMARTS with comma ****")
test_smarts_to_ket(
    "[#6]1-[#6]=[#6]-[#6]=[#6]-[B;r;3;s&2,X3]=1",
    '"queryProperties":{"customQuery":"[B;r;3;s&2,X3]"}',
)
print("**** #1331 wrong smarts for ring bond count as drawn ****")
test_ket_to_smarts("ket_with_rb_as_drawn.ket", "[#6](-[#6])(-[#6;x0])-[#6]")

print("**** #1337 wrong smarts for ring bond count as drawn ****")
fname = "ket_with_custom_query_with_list.ket"
expected = "[#6]1-[#6]=[Cl,Br,I,Na,O]-[#6]=[#6]-[#6]=1"
test_ket_to_smarts(fname, expected)

print(
    "**** #1371 Chirality symbol is added to the SMARTS when "
    "'single up/down' or 'double cis/trans' bond type is set up"
    " wrong smarts for ring bond count as drawn ****"
)
fname = "ket_with_bond_stereo_ether.ket"
expected = r"[#6]1-[#6]=[#6]-[#6]=[#6]\[#6]=1"
test_ket_to_smarts(fname, expected)

print("**** #2036 ket with templates to smarts  ****")
test_ket_to_smarts("5amd.ket", "[#7]1-[#6](-[#7])=[#7]-[#6]2-[#7](-[#6]=[#7]-[#6]=2-[#6]=1-[#7])/[#6@]1-[#6]-[#6@](\\[#8]-[#15](-[H])(-[#8])=[#8])-[#6@@](/[#6]-[#8]-[H])-[#8]-1")
