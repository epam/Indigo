import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
from env_indigo import Indigo, IndigoRenderer, isIronPython, joinPathPy

indigo = Indigo()
renderer = IndigoRenderer(indigo)
indigo.setOption("render-output-format", "svg")
indigo.setOption("render-svg-unique-id", False)

layout_molecules_path = joinPathPy("../layout/molecules/sgroups/", __file__)
molecules_path = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)


def compare_svg(name, structure):
    svg = "".join(chr(b & 0xFF) for b in renderer.renderToBuffer(structure))
    compare_diff(ref_path, "sgroups/" + name + ".svg", svg)


print("*** The structures of layout/sgroup_brackets.py, as loaded ***")
molecules = [
    "brackets-from-file.mol",
    "copolymer-without-brackets.mol",
    "copolymer.ket",
    "datasgroup.mol",
    "f01-sru-straight.mol",
    "f02-sru-zigzag.mol",
    "f03-sru-steep.mol",
    "f04-polystyrene.mol",
    "f05-issue-1969.mol",
    "f06-block-copolymer.mol",
    "f07-mixture.mol",
    "f08-sru-vertical.mol",
    "f09-sru-branching.mol",
    "f10-sru-adjacent-sides.mol",
    "f11-copolymer-in-mixture.mol",
    "f12-mixture-close.mol",
    "f13-repeat-pattern.mol",
    "f14-three-on-one-bond.mol",
    "formulation-without-brackets.mol",
    "gen-sup-zero-next-to-real.mol",
    "mixture-without-brackets.mol",
    "nested-brackets-from-file.mol",
    "rgroup-fragment-sru.mol",
    "sru-without-brackets.mol",
    "ticket-nested-copolymer.mol",
    "ticket-single-sru.mol",
    "zero-brackets.mol",
]
for filename in molecules:
    mol = indigo.loadMoleculeFromFile(
        os.path.join(layout_molecules_path, filename)
    )
    compare_svg(os.path.splitext(filename)[0], mol)

reactions = ["sru-pathway.rdf", "sru-reaction-scaled.rxn", "sru-reaction.rxn"]
for filename in reactions:
    rxn = indigo.loadReactionFromFile(
        os.path.join(layout_molecules_path, filename)
    )
    compare_svg(os.path.splitext(filename)[0], rxn)

print("*** Brackets and labels by the rules of the ticket ***")
# Variants of the structures above: a subtype or repeat pattern changed, or
# the brackets Indigo placed on load swapped, turned with the atoms, joined
# by a third one or reduced to one, which the renderer replaces by a pair
variants = [
    "copolymer-alt",
    "copolymer-ran",
    "copolymer-without-subtype",
    "sru-brackets-horizontal",
    "sru-brackets-swapped",
    "sru-one-bracket",
    "sru-rotated-30",
    "sru-three-brackets",
    "sru-without-scn",
]
for name in variants:
    mol = indigo.loadMoleculeFromFile(
        os.path.join(molecules_path, "sgroups", name + ".mol")
    )
    compare_svg(name, mol)

# CXSMILES carries coordinates but no brackets: the renderer places them
mol = indigo.loadMoleculeFromFile(
    os.path.join(molecules_path, "sgroups", "sru-cxsmiles.smi")
)
compare_svg("sru-cxsmiles", mol)

print("*** Placed brackets follow the drawn bond and the drawn font ***")
mol = indigo.loadMoleculeFromFile(
    os.path.join(molecules_path, "sgroups", "sru-cxsmiles-long-bonds.smi")
)
compare_svg("sru-cxsmiles-long-bonds", mol)
indigo.setOption("render-font-size", 80)
compare_svg("sru-cxsmiles-long-bonds-font-80", mol)
indigo.setOption("render-font-size", -1)
mol = indigo.loadMoleculeFromFile(
    os.path.join(
        molecules_path, "sgroups", "sru-cxsmiles-nested-long-bonds.smi"
    )
)
compare_svg("sru-cxsmiles-nested-long-bonds", mol)

print("*** Hooks of brackets close to each other do not meet ***")
mol = indigo.loadMoleculeFromFile(
    os.path.join(molecules_path, "sgroups", "sru-single-atom.smi")
)
compare_svg("sru-single-atom", mol)

print("*** Atom and bond ids stay those of the molecule ***")
mol = indigo.loadMoleculeFromFile(
    os.path.join(molecules_path, "sgroups", "sru-cxsmiles-with-tail.smi")
)
mol.getAtom(0).remove()
indigo.setOption("render-atom-ids-visible", True)
indigo.setOption("render-bond-ids-visible", True)
compare_svg("sru-ids-after-atom-removal", mol)
indigo.setOption("render-atom-ids-visible", False)
indigo.setOption("render-bond-ids-visible", False)

print("*** A multiple group keeps its brackets and its multiplier ***")
mol = indigo.loadMoleculeFromFile(
    joinPathPy("../formats/molecules/smul.mol", __file__)
)
compare_svg("multiple-group", mol)

print("*** A data S-group without a field name, a colour property set ***")
indigo.setOption("render-atom-color-property", "color")
mol = indigo.loadMoleculeFromFile(
    os.path.join(molecules_path, "sgroups", "data-without-field-name.mol")
)
compare_svg("data-without-field-name", mol)

if isIronPython():
    renderer.Dispose()
    indigo.Dispose()
