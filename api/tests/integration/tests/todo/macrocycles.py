import errno
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

MIN_DIST = 0.1
eps = 0.01

indigo = Indigo()
indigo.setOption("treat-x-as-pseudoatom", "1")
indigo.setOption("smart-layout", "1")
indigo.setOption("molfile-saving-skip-date", "1")

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

print("**** Test Macrocycles ****")

saver = indigo.writeFile(joinPathPy("out/macrocycles.sdf", __file__))

ref_path = getRefFilepath("macrocycles.sdf")
ref = indigo.iterateSDFile(ref_path)
for idx, item in enumerate(
    indigo.iterateSmilesFile(
        joinPathPy("molecules/macrocycles_test.smi", __file__)
    )
):
    try:
        print("Test Item #{} ".format(idx))
        mol = item.clone()
        mol.layout()
        res = moleculeLayoutDiff(
            indigo, mol, ref.at(idx).rawData(), ref_is_file=False
        )
        print("  Result: {}".format(res))
        mol.setProperty("test", "Item #{} ".format(idx))
        saver.sdfAppend(mol)
    except IndigoException as e:
        print("Exception for #%s: %s" % (idx, getIndigoExceptionText(e)))
saver.close()
