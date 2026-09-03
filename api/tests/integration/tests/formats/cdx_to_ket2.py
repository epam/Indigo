import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
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
    ref_filename = os.path.splitext(filename)[0] + ".ket"
    compare_diff(ref_path, ref_filename, ket)
