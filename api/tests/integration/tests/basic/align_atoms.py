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

molecules_path = joinPathPy("molecules/align/", __file__)
ref_path = joinPathPy("ref/", __file__)

# The chain is aligned; the sixth atom of each file stays out of the fit and
# shows where the transform takes a point off the chain
CHAIN_ATOMS = [0, 1, 2, 3, 4]
OFF_CHAIN_ATOM = 5


def load_molecule(name):
    return indigo.loadMoleculeFromFile(
        os.path.join(molecules_path, name + ".mol")
    )


def align(drawn, goal):
    mol = load_molecule(drawn)
    target = load_molecule(goal)
    xyz = []
    for atom in CHAIN_ATOMS:
        xyz.extend(target.getAtom(atom).xyz())
    rms = mol.alignAtoms(CHAIN_ATOMS, xyz)
    print("%s to %s: rms %.3f" % (drawn, goal, rms))
    compare_diff(ref_path, "align/%s-to-%s.mol" % (drawn, goal), mol.molfile())


def load_chain(name):
    mol = load_molecule(name)
    mol.getAtom(OFF_CHAIN_ATOM).remove()
    return mol


def exact_match_within_rms(first, second, rms):
    match = indigo.exactMatch(load_chain(first), load_chain(second), rms)
    print(
        "%s and %s within rms %s: %s" % (first, second, rms, match is not None)
    )


print("*** A chain on a line, on one side of the fit or on both ***")
align("straight-chain", "zigzag-chain")
align("zigzag-chain", "straight-chain")
align("slanted-chain", "zigzag-chain")
align("straight-chain", "reversed-chain")

print("*** A chain off a line: mirrored and scaled ***")
align("zigzag-chain", "zigzag-mirrored")
align("zigzag-chain", "zigzag-scaled")

print("*** An exact match within an rms uses the same fit ***")
exact_match_within_rms("zigzag-chain", "straight-chain", "0.1")
exact_match_within_rms("straight-chain", "zigzag-chain", "0.1")
exact_match_within_rms("straight-chain", "reversed-chain", "0.1")
