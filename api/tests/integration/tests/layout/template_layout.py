import os
import sys
import errno
import math
from math import *

sys.path.append('../../common')
from env_indigo import *

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")
indigo.setOption("treat-x-as-pseudoatom", "1")
indigo.setOption("smart-layout", "1")



ref_path = getRefFilepath("template_layout.sdf")
ref = indigo.iterateSDFile(ref_path)

print("**** Test template layout *****")

saver = indigo.writeFile(joinPathPy("out/template_layout.sdf", __file__))
for idx, item in enumerate(indigo.iterateSDFile(joinPathPy("molecules/template_layout.sdf", __file__))):
    try:
        mol = item.clone()
        mol.layout()
        res = moleculeLayoutDiff(indigo, mol, ref.at(idx).rawData(), ref_is_file = False)
        print("  Item #{}: Result: {}".format(idx, res))
        saver.sdfAppend(mol)
    except IndigoException as e:
        print("Exception for #%s: %s" % (idx, getIntemplate_layout.sdfdigoExceptionText(e)))


print("**** Test rings templates layout *****")

ref_path = getRefFilepath("rings_templates.sdf")
ref = indigo.iterateSDFile(ref_path)

saver = indigo.writeFile(joinPathPy("out/rings_templates.sdf", __file__))
for idx, item in enumerate(ref):
    try:
        mol = item.clone()
        mol.layout()
        res = moleculeLayoutDiff(indigo, mol, item.rawData(), ref_is_file = False)
        print("  Item #{}: Result: {}".format(idx, res))
        saver.sdfAppend(mol)
    except IndigoException as e:
        print("Exception for #%s: %s" % (idx, getIntemplate_layout.sdfdigoExceptionText(e)))