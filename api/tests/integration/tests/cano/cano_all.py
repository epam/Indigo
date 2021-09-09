import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("treat-x-as-pseudoatom", "1")
mol_db_names = [
   (joinPathPy("../../../../../data/molecules/basic/zinc-slice.sdf.gz", __file__), indigo.iterateSDFile),
   (joinPathPy("../../../../../data/molecules/basic/thiazolidines.sdf", __file__), indigo.iterateSDFile),
   (joinPathPy("../../../../../data/molecules/basic/sugars.sdf", __file__), indigo.iterateSDFile),
   (joinPathPy("../../../../../data/molecules/basic/helma.smi", __file__), indigo.iterateSmilesFile),
   (joinPathPy("molecules/cis_trans.smi", __file__), indigo.iterateSmilesFile),
   (joinPathPy("../../../../../data/molecules/stereo/stereo_cis_trans.sdf", __file__), indigo.iterateSDFile),
   (joinPathPy("molecules/set1.sdf", __file__), indigo.iterateSDFile),
   (joinPathPy("molecules/cano_stereocenters.sdf", __file__), indigo.iterateSDFile),
   (joinPathPy("../../../../../data/molecules/basic/tetrahedral-all.cml", __file__), indigo.iterateCMLFile),
   (joinPathPy("molecules/cis_trans_set.sdf", __file__), indigo.iterateSDFile),
   (joinPathPy("molecules/cis_trans_expl_h.sdf", __file__), indigo.iterateSDFile),
   (joinPathPy("molecules/bad_valence.smi", __file__), indigo.iterateSmilesFile),
   (joinPathPy("molecules/arom-ambiguous.smi", __file__), indigo.iterateSmilesFile),
   (joinPathPy("molecules/arom-ambiguous.sdf", __file__), indigo.iterateSDFile),
   (joinPathPy("../../../../../data/molecules/basic/explicit_valence.sdf", __file__), indigo.iterateSDFile),
]

def getNameAndCano (m):
    try:
        name = m.name()
    except IndigoException as e:
        name = getIndigoExceptionText(e)
    try:
        cansm = m.canonicalSmiles()
    except IndigoException as e:
        cansm = getIndigoExceptionText(e)
    try:
        sm = m.smiles()
    except IndigoException as e:
        sm = getIndigoExceptionText(e)
    return name, cansm, sm

for db_name, load_fund in mol_db_names:
    print("Database: %s" % relativePath(db_name))
    idx = 1
    for item in load_fund(db_name):
        name, cansm, sm = getNameAndCano(item)
        print("%s (#%s): %s" % (name, idx, cansm))
        # Check serialization
        try:
            item2 = indigo.unserialize(item.serialize())
            name2, cansm2, sm2 = getNameAndCano(item2)
            if cansm != cansm2:
                print("  Error after serialize: %s: %s" % (name2, cansm2))
                print("     for smiles: %s -> %s" % (sm, sm2))
        except IndigoException as e:
            pass
        idx += 1
