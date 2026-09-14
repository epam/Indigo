import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
from env_indigo import Indigo, joinPathPy  # noqa

print("*** Collection with s-group ***")

indigo = Indigo()
indigo.setOption("molfile-saving-mode", "3000")
indigo.setOption("molfile-saving-skip-date", True)
indigo.setOption("ignore-stereochemistry-errors", True)

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

filename = "collection_sgroup.mol"
fname = os.path.join(root, filename)
molecule = indigo.loadMoleculeFromFile(fname)
mol = molecule.molfile()

compare_diff(ref_path, filename, mol)
