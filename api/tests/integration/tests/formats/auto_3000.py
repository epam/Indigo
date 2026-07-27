import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
from env_indigo import Indigo, joinPathPy  # noqa

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)
indigo.setOption("molfile-saving-mode", "auto")
indigo.setOption("ignore-stereochemistry-errors", True)

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

files = ["cis_trans", "enhanced_stereo3", "atoms (or bonds) exceeds 999"]

files.sort()
for filename in files:
    mol = indigo.loadMoleculeFromFile(os.path.join(root, filename + ".mol"))
    mol_txt = mol.molfile()
    compare_diff(ref_path, filename + ".mol", mol_txt)
