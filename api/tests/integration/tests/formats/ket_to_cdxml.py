import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", True)

print("*** KET to cdxml ***")

root = joinPathPy("molecules/ket", __file__)
files = os.listdir(root)
files.sort()
for filename in files:
    print(filename)
    try:
        mol = indigo.loadMoleculeFromFile(os.path.join(root, filename))
        print(mol.cdxml())
    except IndigoException as e:
        print(getIndigoExceptionText(e))
        try:
            print("*** Try as Reaction ***")
            rct = indigo.loadReactionFromFile(os.path.join(root, filename))
            print(rct.cdxml())
        except IndigoException as e:
            print("*** Try as Query ***")
            mol = indigo.loadQueryMoleculeFromFile(
                os.path.join(root, filename)
            )
            print(mol.cdxml())
