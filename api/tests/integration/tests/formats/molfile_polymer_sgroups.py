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
V2000 = "2000"
V3000 = "3000"
BOTH_VERSIONS = (V2000, V3000)

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)

root = joinPathPy("molecules/" + SUBFOLDER, __file__)
ref_path = joinPathPy("ref/" + SUBFOLDER, __file__)


def load(filename):
    return indigo.loadMoleculeFromFile(os.path.join(root, filename + ".mol"))


def saved(mol, mode):
    indigo.setOption(SAVING_MODE, mode)
    return mol.molfile()


def compare(mol, ref_name, mode):
    compare_diff(ref_path, "%s-%s.mol" % (ref_name, mode), saved(mol, mode))


print("*** Repeat pattern, subtype, component order, nesting ***")
files = [
    # two numbered components; the first holds a head-to-tail block
    # copolymer of two repeating units
    "formulation-v2000",
    # the same kinds of S-groups in V3000, type and subtype spelled out,
    # the copolymer and one repeating unit without CONNECT
    "formulation-words-v3000",
    # every spelling the V3000 reader takes
    "sgroup-words-v3000",
]
for filename in files:
    mol = load(filename)
    for mode in BOTH_VERSIONS:
        compare(mol, filename, mode)
        print("the same after clone()")
        compare(mol.clone(), filename, mode)

print("*** Read back: V2000 -> V3000 -> V2000 ***")
formulation = "formulation-v2000"
from_v3000 = indigo.loadMolecule(saved(load(formulation), V3000))
compare(from_v3000, formulation, V3000)
from_v3000_name = formulation + "-read-from-v3000"
compare(from_v3000, from_v3000_name, V2000)
from_v2000 = indigo.loadMolecule(saved(from_v3000, V2000))
compare(from_v2000, from_v3000_name, V2000)

print("*** Merge into an empty molecule ***")
merged = indigo.createMolecule()
merged.merge(load(formulation))
compare(merged, formulation, V3000)

print("*** Values that are skipped ***")
files = [
    # one component in each file keeps its order; a zero or negative order
    # and an order on a mixture are not taken
    "component-order-not-taken-v2000",
    "component-order-not-taken-v3000",
    # a connectivity code that is not HH, HT or EU leaves either/unknown
    "connectivity-not-known-v2000",
    "connectivity-not-known-v3000",
    # M  SNC names a number M  STY skips, which M  SCN and the other lines
    # skip as well
    "component-order-sgroup-without-sty-v2000",
]
for filename in files:
    mol = load(filename)
    for mode in BOTH_VERSIONS:
        compare(mol, filename, mode)
    for unit in mol.iterateRepeatingUnits():
        print(
            "connectivity of the repeating unit: %d"
            % unit.getRepeatingUnitConnectivity()
        )

print("*** A line that ends early keeps the next one ***")
for filename in [
    # the last order of M  SNC takes two columns instead of three
    "component-order-short-line-v2000",
    # the last code of M  SCN takes one column instead of two
    "connectivity-short-line-v2000",
]:
    mol = load(filename)
    for mode in BOTH_VERSIONS:
        compare(mol, filename, mode)

print("*** The widest order that M  SNC holds, and the next one ***")
for filename in ["component-order-999-v3000", "component-order-1000-v3000"]:
    mol = load(filename)
    for mode in ("auto", V2000):
        compare(mol, filename, mode)

print("*** More entries than one property line holds ***")
nine = "nine-components-nine-copolymers-v3000"
mol = load(nine)
for mode in BOTH_VERSIONS:
    compare(mol, nine, mode)
print("the same after reading the V2000 output")
compare(indigo.loadMolecule(saved(mol, V2000)), nine, V2000)

print("*** Input that is rejected ***")
for filename in [
    "unknown-sgroup-type-v3000",
    "component-order-sgroup-beyond-list-v2000",
    "component-order-fewer-entries-v2000",
]:
    try:
        load(filename)
        print("%s: loaded" % filename)
    except IndigoException as e:
        print("%s: %s" % (filename, getIndigoExceptionText(e)))
