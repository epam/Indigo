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
        print("%s FAILED. Expected string not found." % smarts_in)


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
    "[#6]1-[#6]=[#6]-[#6]=[#6]-[b;r;3;s&2,X3]=1",
    '"queryProperties":{"customQuery":"b;r;3;s&2,X3"}',
)
print("**** #1331 wrong smarts for ring bond count as drawn ****")
test_ket_to_smarts("ket_with_rb_as_drawn.ket", "[#6](-[#6])(-[#6;x0])-[#6]")
