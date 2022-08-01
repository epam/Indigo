import errno
import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

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
        joinPathPy(
            "../../../../../data/molecules/stereo/stereo_cis_trans.sdf",
            __file__,
        ),
        indigo.iterateSDFile,
    ),
    (
        joinPathPy("../../../../../data/molecules/basic/helma.smi", __file__),
        indigo.iterateSmilesFile,
    ),
]

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise


def testAll(clear_cis_trans):
    for db_name, load_fund in mol_db_names:
        print("Database: %s" % relativePath(db_name))
        idx = 1
        db_name_print = os.path.basename(db_name)
        for item in load_fund(db_name):
            try:
                mol = item.clone()

                # for bond in mol.iterateBonds():
                #    if bond.topology() == Indigo.RING and bond.bondOrder() == 2:
                #        bond.resetStereo()

                mol.dearomatize()
                cansm = mol.canonicalSmiles()
                mol2 = indigo.loadMolecule(cansm)
                if clear_cis_trans:
                    mol2.clearCisTrans()
                mol2.layout()
                mol2.markEitherCisTrans()
                mol2.saveMolfile(joinPathPy("out/out.mol", __file__))
                cansm2 = mol2.canonicalSmiles()
                mol3 = indigo.loadMoleculeFromFile(
                    joinPathPy("out/out.mol", __file__)
                )
                cansm3 = mol3.canonicalSmiles()
                if cansm2 != cansm3:
                    sys.stderr.write(
                        "Different canonical smiles for #%s:\n" % idx
                    )
                    sys.stderr.write("  %s\n" % cansm2)
                    sys.stderr.write("  %s\n" % cansm3)
                    sys.stderr.write(
                        "  %s (cansm - before cis-trans removed)\n" % cansm
                    )
                    sys.stderr.write("Database: %s" % relativePath(db_name))
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
                print(
                    "Exception for #%s: %s" % (idx, getIndigoExceptionText(e))
                )
            # sys.stderr.write("%s %s\n" % (db_name_print, idx))
            idx += 1


print("*** With clearing cis-trans ***")
testAll(True)
print("*** Without clearing cis-trans ***")
testAll(False)
