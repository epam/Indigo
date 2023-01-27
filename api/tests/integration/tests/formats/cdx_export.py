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
        # with open(os.path.join(ref_path, filename + ".cdxml"), 'w') as file:
        #    data = file.write(mol.cdxml())
        with open(os.path.join(ref_path, filename + ".b64cdx"), "r") as file:
            refb64 = file.read()
        print(filename + (":success" if refb64 == resb64 else ":failed"))
    except IndigoException as e:
        print(getIndigoExceptionText(e))
        print("*** Try as Query ***")
        mol = indigo.loadQueryMoleculeFromFile(
            os.path.join(root, filename + ".mol")
        )
        resb64 = mol.b64cdx()
        # with open(os.path.join(ref_path, filename + ".cdxml"), 'w') as file:
        #   data = file.write(mol.cdxml())
        with open(os.path.join(ref_path, filename + ".b64cdx"), "r") as file:
            refb64 = file.read()
        print(filename + (":success" if refb64 == resb64 else ":failed"))

cdxml_path = joinPathPy("cdxml/", __file__)
files = os.listdir(cdxml_path)
files.sort()

print("*** CDXML to CDX ***")

for filename in files:
    try:
        mol = indigo.loadReactionFromFile(os.path.join(cdxml_path, filename))
        resb64 = mol.b64cdx()
        # with open(os.path.join(ref_path, filename.rsplit('.', 1)[0] + ".cdxml"), 'w') as file:
        #    data = file.write(mol.cdxml())
        with open(
            os.path.join(ref_path, filename.rsplit(".", 1)[0] + ".b64cdx"), "r"
        ) as file:
            refb64 = file.read()
        print(filename + (":success" if refb64 == resb64 else ":failed"))

    except IndigoException as e:
        print(getIndigoExceptionText(e))
        print("*** Try as Query ***")
        mol = indigo.loadQueryReactionFromFile(
            os.path.join(cdxml_path, filename)
        )
        resb64 = mol.b64cdx()
        # with open(os.path.join(ref_path, filename.rsplit('.', 1)[0] + ".cdxml()"), 'w') as file:
        #    data = file.write(mol.cdxml())
        with open(
            os.path.join(ref_path, filename.rsplit(".", 1)[0] + ".b64cdx"), "r"
        ) as file:
            refb64 = file.read()
        print(filename + (":success" if refb64 == resb64 else ":failed"))
