import sys
import os
import errno

sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("treat-x-as-pseudoatom", "1")
mol_db_names = [
   (joinPath('../../../../../data/molecules/basic/tetrahedral-all.cml'), indigo.iterateCMLFile),
   (joinPath("molecules/helma.smi"), indigo.iterateSmilesFile),
   (joinPath("molecules/arom.sdf"), indigo.iterateSDFile),
   (joinPath("molecules/empty.sdf"), indigo.iterateSDFile),
   (joinPath("molecules/empty1.sdf"), indigo.iterateSDFile)
   ]
   
if not os.path.exists(joinPath("out")):
    try:
        os.makedirs(joinPath("out"))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise
   
f = indigo.writeFile(joinPath("out/out.sdf"))
f2 = open(joinPath("out/cano_out.smi"), "w")
   
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
for item in indigo.iterateSDFile(joinPath("out/out.sdf")):
   try:
      cansm = item.canonicalSmiles()
   except IndigoException as e:
      cansm = getIndigoExceptionText(e)
   print("#%s: %s" % (idx, cansm))
   idx += 1
