import sys
import errno

sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("treat-x-as-pseudoatom", "1")
mol_db_names = [
   (joinPath("../../../../../data/molecules/basic/zinc-slice.sdf.gz"), indigo.iterateSDFile),
   (joinPath("../../../../../data/molecules/basic/thiazolidines.sdf"), indigo.iterateSDFile),
   (joinPath("../../../../../data/molecules/basic/sugars.sdf"),indigo.iterateSDFile),
   (joinPath("molecules/stereo_cis_trans.sdf"),indigo.iterateSDFile),
   (joinPath("../../../../../data/molecules/basic//helma.smi"), indigo.iterateSmilesFile),
   (joinPath("molecules/benzodiazepine_part.sdf"),indigo.iterateSDFile),
]

if not os.path.exists(joinPath("out")):
    try:
        os.makedirs(joinPath("out"))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

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
         mol_f2 = mol2.molfile()
         cansm2 = mol2.canonicalSmiles()
         mol3 = indigo.loadMolecule(mol_f2)
         cansm3 = mol3.canonicalSmiles()
         if cansm2 != cansm3:
            print("Different canonical smiles for #%s:\n" % idx)
            print("  %s\n" % cansm2)
            print("  %s\n" % cansm)
            print("  %s (cansm - before cis-trans removed)\n" % cansm)
            if not os.path.exists(joinPath("bugs")):
               os.mkdir(joinPath("bugs"))
            mol2.saveMolfile(joinPath("bugs/bug_%s_%s_mol2.mol" % (db_name_print, idx)))
            mol3.saveMolfile(joinPath("bugs/bug_%s_%s_mol3.mol" % (db_name_print, idx)))
      except IndigoException as e:
         print("Exception for #%s: %s" % (idx, getIndigoExceptionText(e)))
      idx += 1
