import difflib
import os
import sys


def find_diff(a, b):
    return "\n".join(difflib.unified_diff(a.splitlines(), b.splitlines()))


sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()

print("*** Mol to CDX ***")

root = joinPathPy("molecules/", __file__)
files = [
    "stereo_either-0020",
    "enhanced_stereo1",
    "enhanced_stereo2",
    "enhanced_stereo3",
]

ref_path = joinPathPy("ref/", __file__)
files.sort()

for filename in files:
    try:
        mol = indigo.loadMoleculeFromFile(
            os.path.join(root, filename + ".mol")
        )
        resb64 = mol.b64cdx()
        #        with open(os.path.join(ref_path, filename + ".b64cdx"), 'w') as file:
        #            data = file.write(resb64)
        with open(os.path.join(ref_path, filename + ".b64cdx"), "r") as file:
            refb64 = file.read()
        indigo.loadMolecule(resb64)
        print(filename + (":success" if refb64 == resb64 else ":failed"))
    except IndigoException as e:
        print(getIndigoExceptionText(e))
        print("*** Try as Query ***")
        mol = indigo.loadQueryMoleculeFromFile(
            os.path.join(root, filename + ".mol")
        )
        resb64 = mol.b64cdx()
        #        with open(os.path.join(ref_path, filename + ".b64cdx"), 'w') as file:
        #            data = file.write(resb64)
        with open(os.path.join(ref_path, filename + ".b64cdx"), "r") as file:
            refb64 = file.read()
        indigo.loadQueryMolecule(resb64)
        print(filename + (":success" if refb64 == resb64 else ":failed"))

root = joinPathPy("reactions/", __file__)
files = ["agents"]

files.sort()

for filename in files:
    try:
        rea = indigo.loadReactionFromFile(
            os.path.join(root, filename + ".ket")
        )
        resb64 = rea.b64cdx()
        #        with open(os.path.join(ref_path, filename + ".b64cdx"), 'w') as file:
        #            data = file.write(resb64)
        with open(os.path.join(ref_path, filename + ".b64cdx"), "r") as file:
            refb64 = file.read()
        indigo.loadReaction(resb64)
        print(filename + (":success" if refb64 == resb64 else ":failed"))
    except IndigoException as e:
        print(getIndigoExceptionText(e))
        print("*** Try as Reaction Query ***")
        rea = indigo.loadQueryReactionFromFile(
            os.path.join(root, filename + ".ket")
        )
        resb64 = rea.b64cdx()
        #        with open(os.path.join(ref_path, filename + ".b64cdx"), 'w') as file:
        #            data = file.write(resb64)
        with open(os.path.join(ref_path, filename + ".b64cdx"), "r") as file:
            refb64 = file.read()
        indigo.loadQueryReaction(resb64)
        print(filename + (":success" if refb64 == resb64 else ":failed"))
