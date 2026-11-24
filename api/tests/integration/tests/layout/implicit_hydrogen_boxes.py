import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
from env_indigo import Indigo, joinPathPy

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)

molecules_path = joinPathPy("molecules/implicit_hydrogens/", __file__)
ref_path = joinPathPy("ref/", __file__)


def compare_molfile(name, mol):
    compare_diff(
        ref_path, "implicit_hydrogens/" + name + ".mol", mol.molfile()
    )


print("*** Brackets enclose the hydrogens above and below an atom ***")
# The units have no brackets in the files: they get them on load, by the
# coordinates of the file
files = [
    "sru-amine-peak",
    "sru-amine-valley",
    "sru-ammonium-peak",
    "sru-ammonium-valley",
    "sru-silane-valley",
]
files.sort()
for filename in files:
    mol = indigo.loadMoleculeFromFile(
        os.path.join(molecules_path, filename + ".mol")
    )
    compare_molfile(filename, mol)

print("*** They enclose them again after layout ***")
# layout() draws these units with the middle atom in a valley: a peak file
# would repeat its valley twin
files = [
    "sru-amine-valley",
    "sru-ammonium-valley",
    "sru-silane-valley",
]
for filename in files:
    mol = indigo.loadMoleculeFromFile(
        os.path.join(molecules_path, filename + ".mol")
    )
    mol.layout()
    compare_molfile(filename + "-layout", mol)

print("*** A reaction layout leaves room for the hydrogens of an atom ***")
# A reaction layout measures labels only when it is given a font size
indigo.setOption("render-font-size", 40.0)
rxn = indigo.loadReactionFromFile(
    os.path.join(molecules_path, "reaction-with-water.rxn")
)
rxn.layout()
compare_diff(
    ref_path,
    "implicit_hydrogens/reaction-with-water-layout.rxn",
    rxn.rxnfile(),
)
indigo.setOption("render-font-size", -1.0)
