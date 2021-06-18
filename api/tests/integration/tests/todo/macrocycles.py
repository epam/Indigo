import os
import sys
import errno
import math
from math import *

MIN_DIST = 0.1
eps = 0.01

sys.path.append('../../common')
from env_indigo import *


indigo = Indigo()
indigo.setOption("treat-x-as-pseudoatom", "1")
indigo.setOption("smart-layout", "1")
indigo.setOption("molfile-saving-skip-date", "1")

if not os.path.exists(joinPath("out")):
    try:
        os.makedirs(joinPath("out"))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

print("**** Test Macrocycles ****")

saver = indigo.writeFile(joinPath("out", "macrocycles.sdf"))

ref_path = getRefFilepath("macrocycles.sdf")
ref = indigo.iterateSDFile(ref_path)
for idx, item in enumerate(indigo.iterateSmilesFile(joinPath("molecules", "macrocycles_test.smi"))):
    try:
        print("Test Item #{} ".format(idx))
        mol = item.clone()
        mol.layout()
        res = moleculeLayoutDiff(indigo, mol, ref.at(idx).rawData(), ref_is_file = False)
        print("  Result: {}".format(res))
        mol.setProperty('test', "Item #{} ".format(idx))
        saver.sdfAppend(mol)
    except IndigoException as e:
        print("Exception for #%s: %s" % (idx, getIndigoExceptionText(e)))
saver.close()

