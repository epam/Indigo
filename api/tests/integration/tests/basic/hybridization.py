import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()


def do_test_get_hybridization(molecule_smiles, expecting):
    mol = indigo.loadMolecule(molecule_smiles)

    hybridizations = []
    for atom in mol.iterateAtoms():
        hybridizations.append(atom.getHybridizationStr())
    if hybridizations != expecting:
        print(str(hybridizations) + " != " + str(expecting))


if __name__ == "__main__":
    do_test_get_hybridization(
        "c1ccccc1",
        [
            "SP2",
            "SP2",
            "SP2",
            "SP2",
            "SP2",
            "SP2",
        ],
    )
    do_test_get_hybridization(
        "OC1=CC=CC=C1",
        [
            "SP2",
            "SP2",
            "SP2",
            "SP2",
            "SP2",
            "SP2",
            "SP2",
        ],
    )
    do_test_get_hybridization(
        "[C-]#[O+]",
        ["SP", "SP"],
    ),  # carbon monoxide
    do_test_get_hybridization(
        "O=C=O",
        [
            "SP2",
            "SP",
            "SP2",
        ],
    ),  # carbon dioxide
    do_test_get_hybridization(
        "C#N",
        ["SP", "SP"],
    ),  # hydrogen cyanide
    do_test_get_hybridization(
        "O=C(N)C",  # acetamide
        [
            "SP2",
            "SP2",
            "SP",
            "SP3",
        ],
    ),
    do_test_get_hybridization(
        "OS(=O)(=O)O",  # sulfuric acid
        [
            "SP3",
            "SP3",
            "SP2",
            "SP2",
            "SP3",
        ],
    ),
    do_test_get_hybridization(
        "N(=O)O",
        [
            "SP2",
            "SP2",
            "SP3",
        ],
    ),  # nitrous acid
    do_test_get_hybridization(
        "O=[Xe](=O)(=O)=O",  # xenon tetroxide
        [
            "SP2",
            "SP3",
            "SP2",
            "SP2",
            "SP2",
        ],
    ),
    do_test_get_hybridization(
        "FS(F)(F)(F)(F)F",  # sulfur hexafluoride
        [
            "SP3",
            "SP3D2",
            "SP3",
            "SP3",
            "SP3",
            "SP3",
            "SP3",
        ],
    ),
    do_test_get_hybridization(
        "FBr(F)F",
        [
            "SP3",
            "SP3D",
            "SP3",
            "SP3",
        ],
    ),  # bromine trifluoride
    do_test_get_hybridization(
        "[Be](Cl)Cl",
        [
            "SP",
            "SP3",
            "SP3",
        ],
    ),
    do_test_get_hybridization(
        "C1=CC=CS1",
        [
            "SP2",
            "SP2",
            "SP2",
            "SP2",
            "SP2",
        ],
    )

do_test_get_hybridization(
    "C1=CC=CS1",
    [
        "SP2",
        "SP2",
        "SP2",
        "SP2",
        "SP2",
    ],
)
