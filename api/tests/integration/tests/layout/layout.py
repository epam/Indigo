import errno
import sys

sys.path.append("../../common")
from env_indigo import *

indigo = Indigo()
indigo.setOption("treat-x-as-pseudoatom", "1")
mol_db_names = [
    (
        joinPathPy(
            "../../../../../data/molecules/basic/zinc-slice.sdf.gz", __file__
        ),
        indigo.iterateSDFile,
    ),
    (
        joinPathPy(
            "../../../../../data/molecules/basic/thiazolidines.sdf", __file__
        ),
        indigo.iterateSDFile,
    ),
    (
        joinPathPy("../../../../../data/molecules/basic/sugars.sdf", __file__),
        indigo.iterateSDFile,
    ),
    (
        joinPathPy("molecules/stereo_cis_trans.sdf", __file__),
        indigo.iterateSDFile,
    ),
    (
        joinPathPy("../../../../../data/molecules/basic//helma.smi", __file__),
        indigo.iterateSmilesFile,
    ),
    (
        joinPathPy("molecules/benzodiazepine_part.sdf", __file__),
        indigo.iterateSDFile,
    ),
]

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
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
                if not os.path.exists(joinPathPy("bugs", __file__)):
                    os.mkdir(joinPathPy("bugs", __file__))
                mol2.saveMolfile(
                    joinPathPy(
                        "bugs/bug_%s_%s_mol2.mol" % (db_name_print, idx),
                        __file__,
                    )
                )
                mol3.saveMolfile(
                    joinPathPy(
                        "bugs/bug_%s_%s_mol3.mol" % (db_name_print, idx),
                        __file__,
                    )
                )
        except IndigoException as e:
            print("Exception for #%s: %s" % (idx, getIndigoExceptionText(e)))
        idx += 1
