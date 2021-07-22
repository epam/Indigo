import os
import sys
import errno
import math
from math import *

sys.path.append('../../common')
from env_indigo import *

if not os.path.exists(joinPath("out")):
    try:
        os.makedirs(joinPath("out"))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1");
indigo.setOption("treat-x-as-pseudoatom", "1")
indigo.setOption("smart-layout", "1")



ref_path = getRefFilepath("template_layout.sdf")
ref = indigo.iterateSDFile(ref_path)

print("**** Test template layout *****")

saver = indigo.writeFile(joinPath("out", "template_layout.sdf"))
for idx, item in enumerate(indigo.iterateSDFile(joinPath("molecules", "template_layout.sdf"))):
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

saver = indigo.writeFile(joinPath("out", "rings_templates.sdf"))
for idx, item in enumerate(ref):
    try:
        mol = item.clone()
        mol.layout()
        res = moleculeLayoutDiff(indigo, mol, item.rawData(), ref_is_file = False)
        print("  Item #{}: Result: {}".format(idx, res))
        saver.sdfAppend(mol)
    except IndigoException as e:
        print("Exception for #%s: %s" % (idx, getIntemplate_layout.sdfdigoExceptionText(e)))