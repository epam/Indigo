import os
import re
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

# Allene axial stereo: the central allene carbon of a configured C=C=C is
# labelled P or M during CIP calculation, derived from the terminal-substituent
# priorities and the perceived allene parity. The reference structures
# (Case 17/18 of MAT-75503) come in enantiomeric A/B pairs.

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")


def atom_cips(json_str):
    return re.findall(r'"cip"\s*:\s*"([^"]+)"', json_str)


def cip_sgroups(molfile_str):
    return re.findall(r"\((P|M)\)", molfile_str)


fixtures = "../../../../../data/molecules/allenes"
print("Allene P/M CIP descriptors")
for filename in sorted(os.listdir(joinPathPy(fixtures, __file__))):
    if not filename.startswith("case") or not filename.endswith(".mol"):
        continue

    indigo.setOption("json-saving-add-stereo-desc", "1")
    mol_ket = indigo.loadMoleculeFromFile(
        joinPathPy(fixtures + "/" + filename, __file__)
    )
    ket_cips = atom_cips(mol_ket.json())

    indigo.setOption("molfile-saving-add-stereo-desc", "1")
    mol_mol = indigo.loadMoleculeFromFile(
        joinPathPy(fixtures + "/" + filename, __file__)
    )
    mol_descs = cip_sgroups(mol_mol.molfile())

    print(
        "%s: ket cip=%s molfile sgroup=%s" % (filename, ket_cips, mol_descs)
    )
