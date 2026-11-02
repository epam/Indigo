import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from common.util import compare_diff
from env_indigo import (  # noqa
    Indigo,
    IndigoException,
    getIndigoExceptionText,
    joinPathPy,
)

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)

print("*** KET star atom ***")

root = joinPathPy("molecules/star-atom/", __file__)
ref_path = joinPathPy("ref/star-atom/", __file__)

filename = "sru-star-ends"
mol = indigo.loadMoleculeFromFile(os.path.join(root, filename + ".mol"))
ket = mol.json()
compare_diff(ref_path, filename + ".ket", ket)

reloaded = indigo.loadMolecule(ket)
diff = compare_diff(ref_path, filename + ".ket", reloaded.json(), stdout=False)
print(
    "%s.ket reloaded as a structure:%s"
    % (filename, "FAILED\n" + diff if diff else "SUCCEED")
)

query = indigo.loadQueryMolecule(ket)
compare_diff(ref_path, filename + "-query.smarts", query.smarts())

filename = "sru-any-atom-ends"
try:
    indigo.loadMoleculeFromFile(os.path.join(root, filename + ".ket"))
    print("%s.ket: loaded as a structure" % filename)
except IndigoException as e:
    print("%s.ket: %s" % (filename, getIndigoExceptionText(e)))
