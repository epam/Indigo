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
indigo.setOption("molfile-saving-skip-date", True)
indigo.setOption("json-saving-pretty", True)
indigo.setOption("ket-saving-version", "2.0.0")

print("*** CDX to ket ***")

root = joinPathPy("molecules/cdx", __file__)
ref_path = joinPathPy("ref/", __file__)

files = os.listdir(root)
files.sort()
for filename in files:
    ket = ""
    try:
        mol = indigo.loadMoleculeFromFile(os.path.join(root, filename))
        ket = mol.json()
    except IndigoException as e:
        print(getIndigoExceptionText(e))
        try:
            print("*** Try as Reaction ***")
            rct = indigo.loadReactionFromFile(os.path.join(root, filename))
            ket = rct.json()

        except IndigoException as e:
            print("*** Try as Query ***")
            qmol = indigo.loadQueryMoleculeFromFile(
                os.path.join(root, filename)
            )
            ket = qmol.json()

    with open(
        os.path.join(ref_path, os.path.splitext(filename)[0]) + ".ket",
        "w",
        encoding="utf-8",
    ) as file:
        file.write(ket)

    with open(
        os.path.join(ref_path, os.path.splitext(filename)[0]) + ".ket",
        "r",
        encoding="utf-8",
    ) as file:
        ket_ref = file.read()

    diff = find_diff(ket_ref, ket)
    if not diff:
        print(os.path.splitext(filename)[0] + ".ket:SUCCEED")
    else:
        print(os.path.splitext(filename)[0] + ".ket:FAILED")
        print(diff)
