import os
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("treat-x-as-pseudoatom", "1")
mol_db_names = [
   ("../../data/zinc-slice.sdf", indigo.iterateSDFile),
   ("../../data/thiazolidines.sdf", indigo.iterateSDFile),
   ("../../data/sugars.sdf",indigo.iterateSDFile),
   ("molecules/stereo_cis_trans.sdf",indigo.iterateSDFile),
   ("molecules/helma.smi", indigo.iterateSmilesFile)
]

def testAll (clear_cis_trans):
   for db_name, load_fund in mol_db_names:
      print("Database: %s" % db_name)
      idx = 1
      db_name_print = os.path.basename(db_name)
      for item in load_fund(db_name):
         try:
            mol = item.clone()
            
            for bond in mol.iterateBonds():
               if bond.topology() == Indigo.RING and bond.bondOrder() == 2:
                  bond.resetStereo()
                  
            mol.dearomatize()
            cansm = mol.canonicalSmiles()
            mol2 = indigo.loadMolecule(cansm)
            if clear_cis_trans:
               mol2.clearCisTrans()
            mol2.layout()
            mol2.markEitherCisTrans()
            mol2.saveMolfile("out.mol")
            cansm2 = mol2.canonicalSmiles()
            mol3 = indigo.loadMoleculeFromFile("out.mol")
            cansm3 = mol3.canonicalSmiles()
            if cansm2 != cansm3:
               sys.stderr.write("Different canonical smiles for #%s:\n" % idx)
               sys.stderr.write("  %s\n" % cansm2)
               sys.stderr.write("  %s\n" % cansm3)
               sys.stderr.write("  %s (cansm - before cis-trans removed)\n" % cansm)
               if not os.path.exists("bugs"):
                  os.mkdir("bugs")
               mol2.saveMolfile("bugs/bug_%s_%s_mol2.mol" % (db_name_print, idx))
               mol3.saveMolfile("bugs/bug_%s_%s_mol3.mol" % (db_name_print, idx))
         except IndigoException, e:
            print("Exception for #%s: %s" % (idx, getIndigoExceptionText(e)))
         idx += 1
print("*** With clearing cis-trans ***")
testAll(True)
print("*** Without clearing cis-trans ***")
testAll(False)
