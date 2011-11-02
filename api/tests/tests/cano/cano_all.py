import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("treat-x-as-pseudoatom", "1")
mol_db_names = [ \
   ("../../data/zinc-slice.sdf", indigo.iterateSDFile), \
   ("../../data/thiazolidines.sdf", indigo.iterateSDFile), \
   ("../../data/sugars.sdf", indigo.iterateSDFile), \
   ("molecules/helma.smi", indigo.iterateSmilesFile), \
   ("molecules/cis_trans.smi", indigo.iterateSmilesFile), \
   ("molecules/stereo_cis_trans.sdf", indigo.iterateSDFile), \
   ("molecules/set1.sdf", indigo.iterateSDFile) ]
for db_name, load_fund in mol_db_names:
   print("Database: %s" % (db_name))
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
      idx += 1
