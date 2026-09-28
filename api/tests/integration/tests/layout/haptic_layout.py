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

print("*** Haptic bond layout ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)

# The organometallics collected by the chemist for #3233: hapticity from eta-2 to
# eta-8, sandwiches, chelating and bridging ligands.
files = [
    "allylpalladium-chloride-dimer",
    "bis-1-5-cyclooctadiene-nickel-0",
    "bis-benzene-chromium",
    "bis-cyclopentadienyl-dimethylzirconium-iv",
    "complex-from-kaufmann-et-al-2023",
    "complex-from-seyferth-2002",
    "complex-from-wetzel-roesky-1998",
    "cycloheptatrienyl-cyclopentadienyl-titanium",
    "cycloheptatrienyl-molybdenum-tricarbonyl",
    "cyclobutadiene-iron-tricarbonyl",
    "cyclooctadiene-rhodium-chloride-dimer",
    "cyclopentadienyl-iron-dicarbonyl-dimer",
    "cyclopentadienyl-manganese-tricarbonyl",
    "eta3-allyl-eta5-cyclopentadienyl-palladium",
    "ferrocene",
    "first-complex-from-lu-et-al-2015",
    "pdcl2-bis-diphenylphosphanyl-ferrocene-2",
    "second-complex-from-lu-et-al-2015",
    "tetra-cyclopentadienyl-titanium",
    "third-complex-from-lu-et-al-2015",
    "trans-ir-co-pph3-2-indolyl",
    "tricarbonyl-benzene-chromium",
    "trichloro-ethylene-platinate-ii-hydrate-zeises-salt",
    "triple-decker-complex-from-ghag-at-el-2013",
    "tris-cyclooctatetraene-triiron",
    "tris-cyclopentadienyl-yttrium-iii",
    "uranocene",
]

files.sort()
for filename in files:
    mol = indigo.loadMoleculeFromFile(os.path.join(root, filename + ".ket"))
    mol.layout()
    ket = mol.json()
    compare_diff(ref_path, filename + ".ket", ket)
