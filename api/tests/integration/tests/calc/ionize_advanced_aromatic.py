import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("pKa-model", "advanced")
indigo.setOption("pKa-model-level", 5)
indigo.setOption("pKa-model-min-level", 2)


def ionize(smiles, ph):
    mol = indigo.loadMolecule(smiles)
    try:
        mol.ionize(ph, 0.5)
    except IndigoException as e:
        print("%s: %s" % (smiles, getIndigoExceptionText(e)))
        return None
    print("%s at pH %.1f: %s" % (smiles, ph, mol.canonicalSmiles()))
    return mol


print("*** Advanced pKa model on aromatic input (#1173) ***")

# Aspirin from the issue: the carboxylic acid (pKa about 3.5) loses its
# proton, in the aromatic and in the Kekule form alike
ionize("CC(=O)Oc1ccccc1C(=O)O", 7.4)
ionize("CC(=O)OC1=CC=CC=C1C(=O)O", 7.4)

# Pyridine (pKa about 5.2) at pH 3: the ring nitrogen takes a proton
mol = ionize("c1ccncc1", 3.0)
if mol is not None:
    nitrogen = mol.getAtom(3)
    print(
        "N charge %d, hydrogens %d"
        % (nitrogen.charge(), nitrogen.countImplicitHydrogens())
    )
