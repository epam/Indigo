import os
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("treat-x-as-pseudoatom", "1")
mol_db_names = [
   (joinPath("../../data/zinc-slice.sdf"), indigo.iterateSDFile),
   (joinPath("../../data/thiazolidines.sdf"), indigo.iterateSDFile),
   (joinPath("../../data/sugars.sdf"),indigo.iterateSDFile),
   (joinPath("molecules/stereo_cis_trans.sdf"),indigo.iterateSDFile),
   (joinPath("molecules/helma.smi"), indigo.iterateSmilesFile),
   (joinPath("molecules/benzodiazepine_part.sdf"),indigo.iterateSDFile),
]

if not os.path.exists(joinPath('out')):
    os.makedirs(joinPath('out'))
for db_name, load_fund in mol_db_names:
   print("Database: %s" % relativePath(db_name))
   idx = 1
   db_name_print = os.path.basename(db_name)
   for item in load_fund(db_name):
      try:
         mol = item.clone()
               
         mol.layout()
         mol.dearomatize()
         cansm = mol.canonicalSmiles()
         mol2 = indigo.loadMolecule(cansm)
         mol2.layout()
         mol2.markEitherCisTrans()
         mol2.saveMolfile(joinPath("out/out.mol"))
         cansm2 = mol2.canonicalSmiles()
         mol3 = indigo.loadMoleculeFromFile(joinPath("out/out.mol"))
         cansm3 = mol3.canonicalSmiles()
         if cansm2 != cansm3:
            sys.stderr.write("Different canonical smiles for #%s:\n" % idx)
            sys.stderr.write("  %s\n" % cansm2)
            sys.stderr.write("  %s\n" % cansm)
            sys.stderr.write("  %s (cansm - before cis-trans removed)\n" % cansm)
            if not os.path.exists(joinPath("bugs")):
               os.mkdir(joinPath("bugs"))
            mol2.saveMolfile("bugs/bug_%s_%s_mol2.mol" % (db_name_print, idx))
            mol3.saveMolfile("bugs/bug_%s_%s_mol3.mol" % (db_name_print, idx))
      except IndigoException, e:
         print("Exception for #%s: %s" % (idx, getIndigoExceptionText(e)))
      idx += 1
