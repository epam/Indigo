import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")

print("This test checks if Fischer projection exist")

for root, dirnames, filenames in os.walk(
    joinPathPy("molecules/fischer", __file__)
):
    filenames.sort()
    for filename in filenames:
        mol = indigo.loadMoleculeFromFile(os.path.join(root, filename))
        print("%s" % filename[:-4])
        print(mol.molfile())
        if mol.isPossibleFischerProjection(""):
            print("Possible Fischer projection is found")
        else:
            print("No Ficsher projection is found")
