import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", True)
indigo.setOption("standardize-keep-largest", True)
indigo.setOption("standardize-charges", False)
indigo.setOption("standardize-stereo", True)
indigo.setOption("standardize-neutralize-zwitterions", True)
indigo.setOption("standardize-clear-unusual-valences", True)

m1 = indigo.loadMolecule(
    "ClC(C=C1)=CC=C1NC(NC2=C(C(N)=O)C(CC[N+](CCCCCC(OC(C)(C)C)=O)(C)C3)=C3S2)=O[I-]"
)
m1.standardize()

ii = IndigoInchi(indigo)
inchi_value = ii.getInchi(m1)

# This method fails
m2 = indigo.loadMolecule(inchi_value)
assert ii.getInchi(m2) == inchi_value
