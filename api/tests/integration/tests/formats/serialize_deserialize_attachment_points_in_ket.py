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
indigo.setOption("molfile-saving-mode", 3000)
indigo.setOption("molfile-saving-skip-date", True)

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
        ket = ket_in.json()
        compare_diff(ref_path, filename + ".ket", ket)
    except IndigoException as e:
        print(getIndigoExceptionText(e))

print("*** MOL to MOL ***")
mol_files.sort()
for filename in mol_files:
    mol_in = indigo.loadMoleculeFromFile(os.path.join(root, filename + ".mol"))
    mol = mol_in.molfile()
    compare_diff(ref_path, filename + ".mol", mol)
