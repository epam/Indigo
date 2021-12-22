import errno
import os
import sys

sys.path.append("../../common")
from env_indigo import *

indigo = Indigo()
indigo.setOption("treat-x-as-pseudoatom", "1")
mol_db_names = [
    (
        joinPathPy(
            "../../../../../data/molecules/basic/tetrahedral-all.cml", __file__
        ),
        indigo.iterateCMLFile,
    ),
    (joinPathPy("molecules/helma.smi", __file__), indigo.iterateSmilesFile),
    (joinPathPy("molecules/arom.sdf", __file__), indigo.iterateSDFile),
    (joinPathPy("molecules/empty.sdf", __file__), indigo.iterateSDFile),
    (joinPathPy("molecules/empty1.sdf", __file__), indigo.iterateSDFile),
]

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

f = indigo.writeFile(joinPathPy("out/out.sdf", __file__))
f2 = open(joinPathPy("out/cano_out.smi", __file__), "w")

for db_name, load_fund in mol_db_names:
    print("Database: {0}".format(relativePath(db_name)))
    idx = 1
    for item in load_fund(db_name):
        try:
            name = item.name()
        except IndigoException as e:
            name = getIndigoExceptionText(e)
        try:
            cansm = item.canonicalSmiles()
        except IndigoException as e:
            cansm = getIndigoExceptionText(e)
        print("%s (#%s): %s" % (name, idx, cansm))
        try:
            m2 = item.clone()
            m2.clearProperties()
            m2.dearomatize()
            f.sdfAppend(m2)
            f2.write("%s\n" % m2.canonicalSmiles())
        except IndigoException as e:
            print("save error: %s" % (getIndigoExceptionText(e)))
        idx += 1

f.close()
f2.close()
print("*** Processing result molecules ***")

idx = 1
for item in indigo.iterateSDFile(joinPathPy("out/out.sdf", __file__)):
    try:
        cansm = item.canonicalSmiles()
    except IndigoException as e:
        cansm = getIndigoExceptionText(e)
    print("#%s: %s" % (idx, cansm))
    idx += 1
