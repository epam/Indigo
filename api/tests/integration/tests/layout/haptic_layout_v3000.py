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
indigo.setOption("json-use-native-precision", True)
indigo.setOption("json-set-native-precision", 3)
indigo.setOption("molfile-saving-skip-date", True)
# V2000 has no way to express a haptic bond, so the V3000 mode is not optional.
indigo.setOption("molfile-saving-mode", "3000")

print("*** Haptic bond layout through MOL V3000 ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

# One of each shape the set contains: a sandwich, a chelating ligand, a bridging
# one, an eta-2 end with a stereocentre on the metal, a metal held eight times,
# and the single atom-to-atom bond, which V3000 cannot carry.
files = [
    "bis-1-5-cyclooctadiene-nickel-0",
    "ferrocene",
    "second-complex-from-lu-et-al-2015",
    "trans-ir-co-pph3-2-indolyl",
    "trichloro-ethylene-platinate-ii-hydrate-zeises-salt",
    "tris-cyclooctatetraene-triiron",
]

files.sort()
for filename in files:
    mol = indigo.loadMoleculeFromFile(os.path.join(root, filename + ".ket"))
    mol.layout()
    molfile = mol.molfile()
    compare_diff(ref_path, filename + ".mol", molfile)
    ket = indigo.loadMolecule(molfile).json()
    compare_diff(ref_path, filename + "-from-v3000.ket", ket)
