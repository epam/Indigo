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


def testSmarts(m):
    print(m.smarts())
    print(m.smiles())


def test_smarts_load_save(smarts_in):
    m = indigo.loadSmarts(smarts_in)
    smarts_out = m.smarts()
    if smarts_in == smarts_out:
        print("%s is ok. smarts_in==smarts_out" % smarts_in)
    else:
        print("smarts_in!=smarts_out")
        print(" smarts_in=%s" % smarts_in)
        print("smarts_out=%s" % smarts_out)


def test_smarts_load_save_through_ket(
    smarts_in, expected_str, check_smarts_eq=True
):
    m1 = indigo.loadSmarts(smarts_in)
    json1 = m1.json()
    m2 = indigo.loadQueryMolecule(json1)
    smarts_out = m2.smarts()
    m3 = indigo.loadSmarts(smarts_out)
    json2 = m3.json()
    if check_smarts_eq:
        if smarts_in == smarts_out:
            print("%s is ok. smarts_in==smarts_out" % smarts_in)
        else:
            print("smarts_in!=smarts_out")
            print(" smarts_in=%s" % smarts_in)
            print("smarts_out=%s" % smarts_out)
    if json1 == json2:
        print("%s is ok. json_in==json_out" % smarts_in)
    else:
        print("json_in!=json_out")
        print(" json_in=%s" % json1)
        print("json_out=%s" % json2)
    if expected_str in json1:
        print("%s is ok. expected string found in json" % smarts_in)
    else:
        print("%s: expected string not found in json" % smarts_in)
        print(json1)


molstr = """
  Ketcher 11241617102D 1   1.00000     0.00000     0

  8  7  0     0  0            999 V2000
    3.7000   -4.9000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    4.5660   -5.4000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.4321   -4.9000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.2981   -5.4000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.1641   -4.9000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.0301   -5.4000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.8962   -4.9000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.7622   -5.4000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0     0  0
  2  3  1  0     0  0
  3  4  1  0     0  0
  4  5  1  0     0  0
  5  6  1  0     0  0
  6  7  1  0     0  0
  7  8  1  0     0  0
M  CHG  1   3   5
M  END
"""

notlist = """
  -INDIGO-06202202172D

  0  0  0  0  0  0  0  0  0  0  0 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 9 8 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 4.9359 -3.675 0.0 0
M  V30 2 C 5.80192 -3.175 0.0 0
M  V30 3 NOT[B,C,N] 6.66795 -3.675 0.0 0
M  V30 4 C 7.53397 -3.175 0.0 0
M  V30 5 C 8.4 -3.675 0.0 0
M  V30 6 C 9.26603 -3.175 0.0 0
M  V30 7 C 10.1321 -3.675 0.0 0
M  V30 8 C 10.9981 -3.175 0.0 0
M  V30 9 C 11.8641 -3.675 0.0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 2 1 2 3
M  V30 3 1 3 4
M  V30 4 1 4 5
M  V30 5 1 5 6
M  V30 6 1 6 7
M  V30 7 1 7 8
M  V30 8 1 8 9
M  V30 END BOND
M  V30 END CTAB
M  END
"""

print("**** Load and Save as Query ****")
m = indigo.loadQueryMolecule(molstr)
testSmarts(m)

print("**** Load and Save as Molecule ****")
m = indigo.loadMolecule(molstr)
testSmarts(m)

print("**** Load and Save as Query with not list ****")
m = indigo.loadQueryMolecule(notlist)
print(m.smarts())

print("**** Load and Save as Query with component-level grouping ****")
test_smarts_load_save("([#8].[#6])")
test_smarts_load_save("([#8].[#6]).([#8].[#6])")

test_smarts_load_save("[!C;!b]")
test_smarts_load_save("[*]")
test_smarts_load_save("[*;R1]")
test_smarts_load_save("[*;R3]")
test_smarts_load_save("[r]")
test_smarts_load_save("[r0]")
test_smarts_load_save("[r1]")
test_smarts_load_save("[r3]")
test_smarts_load_save("[v]")
test_smarts_load_save("[v0]")
test_smarts_load_save("[v3]")
test_smarts_load_save("[+0]")
test_smarts_load_save("[#6]@[#6]")
test_smarts_load_save("[#9]/[#6]")
test_smarts_load_save("[#9]/[#6]=[C]/[#17]")
test_smarts_load_save("[O;H]")
test_smarts_load_save("[!O;H]")
test_smarts_load_save("([#6]1-[#6]-[#6]-1.[#6])")
test_smarts_load_save("[#9]/[#6]=[#6]/[#6]=[#6]/[#6]")
test_smarts_load_save("[#9]\[#6]=[#6]\[#6]=[#6]\[#6]")
expected_str = '"bonds":[{"type":1,"atoms":[0,1],"stereo":1},{"type":2,"atoms":[1,2]},{"type":1,"atoms":[2,3],"stereo":1},{"type":2,"atoms":[3,4]},{"type":1,"atoms":[4,5],"stereo":1}]}}'
test_smarts_load_save_through_ket(
    "[#9]/[#6]=[#6]/[#6]=[#6]/[#6]", expected_str
)
expected_str = '"bonds":[{"type":1,"atoms":[0,1],"stereo":6},{"type":2,"atoms":[1,2]},{"type":1,"atoms":[2,3],"stereo":1},{"type":2,"atoms":[3,4]},{"type":1,"atoms":[4,5],"stereo":6}]}}'
test_smarts_load_save_through_ket(
    "[#9]\[#6]=[#6]/[#6]=[#6]\[#6]", expected_str
)
expected_str = '"bonds":[{"customQuery":"\\\?","atoms":[0,1]},{"type":2,"atoms":[1,2]},{"customQuery":"/?","atoms":[2,3]},{"type":2,"atoms":[3,4]},{"customQuery":"\\\?","atoms":[4,5]}]'
test_smarts_load_save_through_ket(
    "[#9]\?[#6]=[#6]/?[#6]=[#6]\?[#6]", expected_str
)
test_smarts_load_save_through_ket(
    "[C;@OH2]",
    '"atoms":[{"label":"C","location":[0.0,0.0,0.0],"queryProperties":{"customQuery":"C;@OH2"}}]',
)
test_smarts_load_save_through_ket(
    "[C;@]",
    '"atoms":[{"label":"C","location":[0.0,0.0,0.0],"queryProperties":{"aromaticity":"aliphatic","chirality":"anticlockwise"}}]',
)
test_smarts_load_save_through_ket(
    "[C;@@]",
    '"atoms":[{"label":"C","location":[0.0,0.0,0.0],"queryProperties":{"aromaticity":"aliphatic","chirality":"clockwise"}}]',
)
test_smarts_load_save_through_ket(
    "[#6]-;@[#6]", '"bonds":[{"type":1,"topology":1,"atoms":[0,1]}]'
)
test_smarts_load_save_through_ket(
    "[#6]-;!@[#6]", '"bonds":[{"type":1,"topology":2,"atoms":[0,1]}]'
)
print("#1292 test smarts c:1-2:c(:c:c:c:c:1)-[#6](=[#8])-[#6;X4]-[#6]-2=[#8]")
indigo.loadSmarts("c:1-2:c(:c:c:c:c:1)-[#6](=[#8])-[#6;X4]-[#6]-2=[#8]")
print("smarts loaded OK")
test_smarts_load_save_through_ket(
    "[!#6,!#7,!#8]", '"queryProperties":{"customQuery":"!#6,!#7,!#8"}}]'
)
test_smarts_load_save_through_ket(
    "[!#6!#7!#8]",
    '"atoms":[{"type":"atom-list","notList":true,"elements":["C","N","O"],"location":[0.0,0.0,0.0]}]',
)
print("#1355 Error appeared at save query molecule with RSite as smarts")
m = indigo.loadQueryMolecule(
    "S=CC(F)CCCCC[C@@](CCO)/C=C/[C@@](N)CCC[C]C([13C]CC([C+2]CC(CC%91)CC(C)CCC)CCC)CC%92.[*:2]%92.[*:1]%91 |$;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;_R2;_R1$,rb:32:*,u:3|"
)
smarts = m.smarts()
expected = "[#16]=[#6]-[#6](-[#6]-[#6]-[#6]-[#6]-[#6]-[#6@](-[#6]=[#6]-[#6@](-[#6]-[#6]-[#6]-[#6]-[#6](-[#6]-[#6]%91)-[13;#6]-[#6]-[#6](-[#6]-[#6]-[#6])-[#6;+2]-[#6]-[#6](-[#6]-[#6](-[#6]-[#6]-[#6])-[#6;x0])-[#6]-[#6]%92)-[#7])-[#6]-[#6]-[#8])-[#9;$([*,#1]=,#,:[*,#1])].[*:2]-%91.[*:1]-%92"
if smarts == expected:
    print("Ok expected smarts generated")
else:
    print(
        "Fail. Expected smarts is\n%s\nbut generated\n%s" % (expected, smarts)
    )
smarts = "[!#40!#79!#30]-[#6]-[#6]"
expected = (
    '"atoms":[{"type":"atom-list","notList":true,"elements":["Zr","Au","Zn"],'
)
test_smarts_load_save_through_ket(smarts, expected)
smarts = "[#40,#79,#30]-[#6]-[#6]"
expected = '"atoms":[{"type":"atom-list","elements":["Zr","Au","Zn"],'
test_smarts_load_save_through_ket(smarts, expected)
smarts = "[#6,#7;a]:[o]"
expected = '"atoms":[{"type":"atom-list","elements":["C","N"],"location":[0.0,0.0,0.0],"queryProperties":{"aromaticity":"aromatic"}}'
test_smarts_load_save_through_ket(smarts, expected)
smarts = "[c,n,o]:[o]"
expected = '"atoms":[{"type":"atom-list","elements":["C","N","O"],"location":[0.0,0.0,0.0],"queryProperties":{"aromaticity":"aromatic"}}'
test_smarts_load_save_through_ket(smarts, expected, False)
smarts = "[c,C,c]"
expected = '"atoms":[{"label":"C","location":[0.0,0.0,0.0],"queryProperties":{"customQuery":"c,C,c"}}]'
test_smarts_load_save_through_ket(smarts, expected)
smarts = "[C,c]"
expected = '"atoms":[{"label":"C","location":[0.0,0.0,0.0],"queryProperties":{"customQuery":"C,c"}}]'
test_smarts_load_save_through_ket(smarts, expected)
smarts = "[C,c,n,o]"
expected = '"atoms":[{"label":"","location":[0.0,0.0,0.0],"queryProperties":{"customQuery":"C,c,n,o"}}]'
test_smarts_load_save_through_ket(smarts, expected)
smarts = "[#7;v4]-[#6]"
expected = '"atoms":[{"label":"N","location":[0.0,0.0,0.0],"explicitValence":4},{"label":"C","location":[1.0,0.0,0.0]}]'
test_smarts_load_save_through_ket(smarts, expected)
print(" *** SMARTS specific ***")
smarts_list = [
    "[C,N]",
    "[C;N]",
    "[C&N]",
    "[!C]",
    "[$(C)]",
    "[C]!-[C]",
    "[C]-,=[C]",
    "[C]-;=[C]",
    "[C]-&=[C]",
]
for smarts in smarts_list:
    qm = indigo.loadQueryMolecule(smarts)
    format = qm.getOriginalFormat()
    if format == "chemical/x-daylight-smarts":
        print("Smarts %s loaded as smarts - OK." % smarts)
    else:
        print("Smarts %s loaded as %s - FAILED" % (smarts, format))
