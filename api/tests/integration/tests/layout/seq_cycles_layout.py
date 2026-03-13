import json
import os
import sys


def compare_positions(ket_a, ket_b, eps=0.05):
    """Compare two KET JSONs, allowing epsilon tolerance on position coordinates."""
    a = json.loads(ket_a)
    b = json.loads(ket_b)
    nodes_a = a.get("root", {}).get("nodes", [])
    nodes_b = b.get("root", {}).get("nodes", [])
    if len(nodes_a) != len(nodes_b):
        return "Node count mismatch: {} vs {}".format(
            len(nodes_a), len(nodes_b)
        )
    for na, nb in zip(nodes_a, nodes_b):
        if na.get("type") != nb.get("type"):
            return "Node type mismatch: {} vs {}".format(
                na.get("type"), nb.get("type")
            )
        pa = na.get("position")
        pb = nb.get("position")
        if pa and pb:
            dx = abs(pa["x"] - pb["x"])
            dy = abs(pa["y"] - pb["y"])
            if dx > eps or dy > eps:
                return (
                    "Position mismatch for id={}: "
                    "({:.4f}, {:.4f}) vs "
                    "({:.4f}, {:.4f}), "
                    "delta=({:.4f}, {:.4f})"
                ).format(
                    na.get("id"), pa["x"], pa["y"], pb["x"], pb["y"], dx, dy
                )
    return ""


sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)

from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)
indigo.setOption("ignore-stereochemistry-errors", True)
indigo.setOption("json-use-native-precision", True)
indigo.setOption("json-set-native-precision", 4)

print("*** Sequence cycles layout ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)


files = [
    "pep_sel",
    "rna_sel",
    "n-agon1",
    "n-agon2",
    "rna_bicyclic",
    "rna_bicyclic_sel_2",
    "small_mol",
    "shifting_structs_1",
    "shifting_structs_2",
    "shifting_structs_1_sel",
    "shifting_structs_2_sel",
    "overlapping",
    "ring_fuse",
    "left_top_monomer",
    "6bases",
]

files.sort()
for filename in files:
    try:
        mol = indigo.loadMoleculeFromFile(
            os.path.join(root, filename + ".ket")
        )
    except:
        mol = indigo.loadQueryMoleculeFromFile(
            os.path.join(root, filename + ".ket")
        )

    mol.layout()
    # with open(os.path.join(ref_path, filename) + ".ket", "w") as file:
    #     file.write(mol.json())
    with open(getRefFilepath(filename + ".ket"), "r") as file:
        ket_ref = file.read()

    ket = mol.json()
    diff = compare_positions(ket_ref, ket)
    if not diff:
        print(filename + ".ket:SUCCEED")
    else:
        print(filename + ".ket:FAILED")
        print(diff)
