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


def load_molecule(filename):
    return indigo.loadMoleculeFromFile(os.path.join(molecules_path, filename))


def load_reaction(filename):
    return indigo.loadReactionFromFile(os.path.join(molecules_path, filename))


def compare_molfile(name, mol):
    compare_diff(ref_path, "sgroups/" + name + ".mol", mol.molfile())


def rdf(pathway):
    buffer = indigo.writeBuffer()
    saver = indigo.createSaver(buffer, "rdf")
    for reaction in pathway.iterateReactions():
        saver.append(reaction.clone())
    saver.close()
    return buffer.toString()


print("*** A reaction or a pathway layout carries the data values ***")
# The molecules kept as drawn are scaled to the default bond length: an
# absolute value keeps its place by the atoms, a relative offset is scaled
indigo.setOption("layout-preserve-existing", True)
rxn = load_reaction("data-reaction-scaled.rxn")
rxn.layout()
compare_diff(
    ref_path,
    "sgroups/data-reaction-scaled-layout-preserve.rxn",
    rxn.rxnfile(),
)
# The value of a single atom gets its scale from the move, not from a fit
rxn = load_reaction("data-reaction-one-atom.rxn")
rxn.layout()
compare_diff(
    ref_path, "sgroups/data-reaction-one-atom-layout.rxn", rxn.rxnfile()
)
indigo.setOption("layout-preserve-existing", False)

# Laid out anew, a molecule has its values fitted to its new atoms first, then
# carried with it into the scheme
rxn = load_reaction("data-reaction-scaled.rxn")
rxn.layout()
compare_diff(
    ref_path, "sgroups/data-reaction-scaled-layout.rxn", rxn.rxnfile()
)

# Loading a pathway scales its molecules and places them in the scheme
pathway = load_reaction("data-pathway.rdf")
compare_diff(ref_path, "sgroups/data-pathway.rdf", rdf(pathway))

print("*** Layout and clean-up keep the data values by their atoms ***")
# The structures of the ticket; one molecule drawn turned, mirrored and at other
# bond lengths; groups on a straight line before or after the layout. A second
# layout() leaves the values where the first one put them
files = [
    "data-bent-nitrile",
    "data-chain-mirrored",
    "data-chain-scaled",
    "data-chain-shrunk",
    "data-chain-turned",
    "data-on-component",
    "data-straight-chain",
    "data-vertical-copolymer",
]
files.sort()
for filename in files:
    mol = load_molecule(filename + ".mol")
    mol.layout()
    compare_molfile(filename + "-layout", mol)
    mol.layout()
    compare_molfile(filename + "-layout", mol)

mol = load_molecule("data-distorted-copolymer.mol")
mol.clean2d()
compare_molfile("data-distorted-copolymer-clean2d", mol)

# A clean-up of the atoms of one value leaves the other value bit for bit
mol = load_molecule("data-distorted-copolymer.mol")
mol.getSubmolecule([4, 5, 6, 7, 8]).clean2d()
compare_molfile("data-distorted-copolymer-clean2d-selection", mol)
