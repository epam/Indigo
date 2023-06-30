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
indigo.setOption("molfile-saving-mode", 3000)


root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

ket_files = [
    "super_atom_attachment_point_wo_leav_pnt",
    "super_atom_attachment_point_w_leav_pnt",
    "super_atom_attachment_point_w_leav_pnt_wo_mandatory",
]

mol_files = [
    "super_atom_attachment_point_w_leav_pnt",
    "super_atom_attachment_point_wo_leav_pnt",
]

print("*** KET to KET ***")
ket_files.sort()
for filename in ket_files:
    try:
        ket_in = indigo.loadMoleculeFromFile(
            os.path.join(root, filename + ".ket")
        )
        # print(ket_in.json())
        with open(
            os.path.join(ref_path, filename) + "_out" + ".ket", "w"
        ) as file:
            file.write(ket_in.json())
        with open(
            os.path.join(ref_path, filename) + "_out" + ".ket", "r"
        ) as file:
            ket_ref = file.read()
        ket = ket_in.json()
        diff = find_diff(ket_ref, ket)
        if not diff:
            print(filename + ".ket:SUCCEED")
        else:
            print(filename + ".ket:FAILED")
            print(diff)
    except IndigoException as e:
        print(getIndigoExceptionText(e))

print("*** MOL to MOL ***")
mol_files.sort()
for filename in mol_files:
    mol_in = indigo.loadMoleculeFromFile(os.path.join(root, filename + ".mol"))
    # print(ket_in.json())
    with open(os.path.join(ref_path, filename) + "_out" + ".mol", "w") as file:
        file.write(mol_in.molfile())
    with open(os.path.join(ref_path, filename) + "_out" + ".mol", "r") as file:
        mol_ref = file.read()
    # mol_ket = mol_in.json()
    # print("mol_ref : ", mol_ref)
    # print("mol_ref : ", mol_ref)
    diff = find_diff(mol_ref, mol_in.molfile())
    if not diff:
        print(filename + ".mol:SUCCEED")
    else:
        print(filename + ".mol:FAILED")
        print(diff)
