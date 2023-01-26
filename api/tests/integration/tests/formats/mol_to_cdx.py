import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()

print("*** Mol to CDXML ***")

root = joinPathPy("molecules/", __file__)
files = [
    "stereo_either-0020.mol",
    "enhanced_stereo1.mol",
    "enhanced_stereo2.mol",
    "enhanced_stereo3.mol",
]
files.sort()
for filename in files:
    print(filename)
    try:
        mol = indigo.loadMoleculeFromFile(os.path.join(root, filename))
        print(filename)
        print(mol.b64cdx())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
        print("*** Try as Query ***")
        mol = indigo.loadQueryMoleculeFromFile(os.path.join(root, filename))
        print(mol.b64cdx())
