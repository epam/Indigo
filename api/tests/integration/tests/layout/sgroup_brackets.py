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

molecules_path = joinPathPy("molecules/sgroups/", __file__)
ref_path = joinPathPy("ref/", __file__)


def load(filename):
    return indigo.loadMoleculeFromFile(os.path.join(molecules_path, filename))


def compare_molfile(name, mol):
    compare_diff(ref_path, "sgroups/" + name + ".mol", mol.molfile())


print("*** Loaded brackets stay while their atoms stay ***")
# The copolymer keeps its brackets though its repeating units get new ones
mol = load("nested-brackets-from-file.mol")
compare_molfile("nested-brackets-from-file", mol)

# Indices 3 to 6, atoms 4 to 7 in the file: the second repeating unit and
# its `*` ends, a crooked chain
mol = load("brackets-from-file.mol")
compare_molfile("brackets-from-file", mol)
mol.getSubmolecule([3, 4, 5, 6]).layout()
compare_molfile("brackets-from-file-layout-selection", mol)

mol = load("brackets-from-file.mol")
mol.getSubmolecule([3, 4, 5, 6]).clean2d()
compare_molfile("brackets-from-file-clean2d-selection", mol)

mol = load("brackets-from-file.mol")
mol.clean2d()
compare_molfile("brackets-from-file-clean2d", mol)

print("*** Bracketed groups without brackets get them on load and layout ***")
# A second layout() keeps the brackets of the first one
files = [
    "copolymer-without-brackets",
    "formulation-without-brackets",
    "mixture-without-brackets",
    "sru-without-brackets",
    "zero-brackets",
]
files.sort()
for filename in files:
    mol = load(filename + ".mol")
    compare_molfile(filename, mol)
    mol.layout()
    compare_molfile(filename + "-layout", mol)
    mol.layout()
    compare_molfile(filename + "-layout", mol)

mol = load("copolymer.ket")
compare_molfile("copolymer-from-ket", mol)

print("*** Zero-length brackets are not written ***")
indigo.setOption("molfile-saving-mode", "3000")
compare_molfile("zero-brackets-v3000", load("zero-brackets.mol"))
indigo.setOption("molfile-saving-mode", "auto")

print("*** The mock-up structures, as loaded and after layout() ***")
mockups = [
    "f01-sru-straight",
    "f02-sru-zigzag",
    "f03-sru-steep",
    "f04-polystyrene",
    "f05-issue-1969",
    "f06-block-copolymer",
    "f07-mixture",
    "f08-sru-vertical",
    "f09-sru-branching",
    "f10-sru-adjacent-sides",
    "f11-copolymer-in-mixture",
    "f12-mixture-close",
    "f13-repeat-pattern",
    "f14-three-on-one-bond",
]
for filename in mockups:
    mol = load(filename + ".mol")
    compare_molfile(filename, mol)
    mol.layout()
    compare_molfile(filename + "-layout", mol)

print(
    "*** Labels are measured at the font size and label mode of the options ***"
)
indigo.setOption("render-font-size", 80.0)
mol = load("f05-issue-1969.mol")
mol.layout()
compare_molfile("f05-issue-1969-layout-font-80", mol)
indigo.setOption("render-font-size", -1.0)

indigo.setOption("render-label-mode", "all")
mol = load("f05-issue-1969.mol")
mol.layout()
compare_molfile("f05-issue-1969-layout-label-mode-all", mol)
rxn = indigo.loadReactionFromFile(
    os.path.join(molecules_path, "sru-reaction.rxn")
)
rxn.layout()
compare_diff(
    ref_path, "sgroups/sru-reaction-layout-label-mode-all.rxn", rxn.rxnfile()
)
indigo.setOption("render-label-mode", "terminal-hetero")

print("*** Brackets move with the molecules of a reaction and a pathway ***")
rxn = indigo.loadReactionFromFile(
    os.path.join(molecules_path, "sru-reaction.rxn")
)
rxn.layout()
compare_diff(ref_path, "sgroups/sru-reaction-layout.rxn", rxn.rxnfile())

indigo.setOption("layout-preserve-existing", True)
rxn = indigo.loadReactionFromFile(
    os.path.join(molecules_path, "sru-reaction.rxn")
)
rxn.layout()
compare_diff(
    ref_path, "sgroups/sru-reaction-layout-preserve.rxn", rxn.rxnfile()
)
indigo.setOption("layout-preserve-existing", False)

pathway = indigo.loadReactionFromFile(
    os.path.join(molecules_path, "sru-pathway.rdf")
)
buffer = indigo.writeBuffer()
saver = indigo.createSaver(buffer, "rdf")
for reaction in pathway.iterateReactions():
    saver.append(reaction.clone())
saver.close()
compare_diff(ref_path, "sgroups/sru-pathway.rdf", buffer.toString())
