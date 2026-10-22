import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
from env_indigo import *  # noqa

SUBFOLDER = "sgroup"
SAVING_MODE = "molfile-saving-mode"
BOTH_VERSIONS = ("2000", "3000")

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)
indigo.setOption("json-saving-pretty", True)
indigo.setOption("json-use-native-precision", True)
indigo.setOption("json-set-native-precision", 3)

root = joinPathPy("molecules/" + SUBFOLDER, __file__)
ref_path = joinPathPy("ref/" + SUBFOLDER, __file__)


def path(name):
    return os.path.join(root, name)


def compare_molfiles(mol, ref_name):
    for mode in BOTH_VERSIONS:
        indigo.setOption(SAVING_MODE, mode)
        compare_diff(ref_path, "%s-%s.mol" % (ref_name, mode), mol.molfile())


print("*** MOL -> KET -> MOL ***")
for filename in [
    # two numbered components of a formulation; the first holds a block
    # copolymer of two repeating units
    "formulation-v2000",
    "formulation-words-v3000",
    # a copolymer around a multiple group: the repeats join the copolymer,
    # they do not copy it
    "copolymer-around-multiple-v3000",
    # a generic S-group on two carbons that a monomer joins: both carbons
    # go to one molecule node, and the S-group stays one
    "generic-across-monomer-v3000",
]:
    ket = indigo.loadMoleculeFromFile(path(filename + ".mol")).json()
    compare_diff(ref_path, filename + ".ket", ket)
    compare_molfiles(indigo.loadMolecule(ket), filename + "-ket")

print("*** KET as Ketcher writes it, without parentId ***")
for filename in [
    "copolymer-without-parent-ids",
    "mixture-over-two-fragments",
    "formulation-over-two-fragments",
    # a data S-group is nobody's parent; equal atom sets give no parent;
    # of two equal parents the first one is taken. The data S-group
    # with neither atoms nor a parent is lost when the nodes are joined,
    # as before
    "data-and-generic-in-unit",
]:
    compare_molfiles(
        indigo.loadMoleculeFromFile(path(filename + ".ket")), filename
    )

print("*** KET with parentId ***")
for filename in [
    "data-without-atoms-with-parent-id",
    # the first node has no parentId of its own, but the document has one
    "nesting-in-second-node",
    "rgroup-fragment-without-parent-ids",
    # the only parentId of the document is in a fragment of an R-group
    "rgroup-fragments-name-the-parent",
]:
    compare_molfiles(
        indigo.loadMoleculeFromFile(path(filename + ".ket")), filename
    )

print("*** Input that is rejected ***")
for filename, load in [
    ("rejected-type-lowercase", indigo.loadMoleculeFromFile),
    ("rejected-type-mon", indigo.loadMoleculeFromFile),
    ("rejected-type-not-string", indigo.loadMoleculeFromFile),
    ("rejected-atoms-missing", indigo.loadMoleculeFromFile),
    ("rejected-atom-out-of-range", indigo.loadMoleculeFromFile),
    ("rejected-atom-negative", indigo.loadMoleculeFromFile),
    ("rejected-bond-out-of-range", indigo.loadMoleculeFromFile),
    (
        "rejected-query-component-atom-out-of-range",
        indigo.loadQueryMoleculeFromFile,
    ),
    ("rejected-multiplier-zero", indigo.loadMoleculeFromFile),
    ("rejected-multiplier-1001", indigo.loadMoleculeFromFile),
    ("rejected-parent-id-unknown", indigo.loadMoleculeFromFile),
    ("rejected-parent-id-cycle", indigo.loadMoleculeFromFile),
    ("rejected-id-repeated", indigo.loadMoleculeFromFile),
    ("rejected-id-negative", indigo.loadMoleculeFromFile),
    ("rejected-component-order-on-mixture", indigo.loadMoleculeFromFile),
    ("rejected-component-order-zero", indigo.loadMoleculeFromFile),
    (
        "rejected-query-component-with-parent-id",
        indigo.loadQueryMoleculeFromFile,
    ),
    ("rejected-connectivity-not-string", indigo.loadMoleculeFromFile),
]:
    try:
        load(path(filename + ".ket"))
        print("%s: loaded" % filename)
    except IndigoException as e:
        print("%s: %s" % (filename, getIndigoExceptionText(e)))
