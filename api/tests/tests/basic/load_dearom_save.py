import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("treat-x-as-pseudoatom", "1")
mol_db_names = [
   ("../../data/tetrahedral-all.cml", indigo.iterateCMLFile),
   ("molecules/helma.smi", indigo.iterateSmilesFile),
   ("molecules/arom.sdf", indigo.iterateSDFile),
   ("molecules/empty.sdf", indigo.iterateSDFile),
   ("molecules/empty1.sdf", indigo.iterateSDFile)
   ]
f = indigo.writeFile("out.sdf")
f2 = open("cano_out.smi", "w")
   
for db_name, load_fund in mol_db_names:
   print("Database: %s" % db_name)
   idx = 1
   for item in load_fund(db_name):
      try:
         name = item.name()
      except IndigoException, e:
         name = getIndigoExceptionText(e)
      try:
         cansm = item.canonicalSmiles()
      except IndigoException, e:
         cansm = getIndigoExceptionText(e)
      print("%s (#%s): %s" % (name, idx, cansm))
      try:
         m2 = item.clone()
         m2.clearProperties()
         m2.dearomatize()
         f.sdfAppend(m2)
         f2.write("%s\n" % m2.canonicalSmiles())
      except IndigoException, e:
         print("save error: %s" % (getIndigoExceptionText(e)))
      idx += 1
      
f.close()
f2.close()
print("*** Processing result molecules ***")
   
idx = 1
for item in indigo.iterateSDFile("out.sdf"):
   try:
      cansm = item.canonicalSmiles()
   except IndigoException, e:
      cansm = getIndigoExceptionText(e)
   print("#%s: %s" % (idx, cansm))
   idx += 1
   
