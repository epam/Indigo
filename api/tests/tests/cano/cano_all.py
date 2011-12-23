import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("treat-x-as-pseudoatom", "1")
mol_db_names = [
   (joinPath("../../data/zinc-slice.sdf"), indigo.iterateSDFile),
   (joinPath("../../data/thiazolidines.sdf"), indigo.iterateSDFile),
   (joinPath("../../data/sugars.sdf"), indigo.iterateSDFile),
   (joinPath("molecules/helma.smi"), indigo.iterateSmilesFile),
   (joinPath("molecules/cis_trans.smi"), indigo.iterateSmilesFile),
   (joinPath("molecules/stereo_cis_trans.sdf"), indigo.iterateSDFile),
   (joinPath("molecules/set1.sdf"), indigo.iterateSDFile),
   (joinPath("molecules/cano_stereocenters.sdf"), indigo.iterateSDFile),
   (joinPath("../../data/tetrahedral-all.cml"), indigo.iterateCMLFile),
   (joinPath("molecules/cis_trans_set.sdf"), indigo.iterateSDFile),
   (joinPath("molecules/cis_trans_expl_h.sdf"), indigo.iterateSDFile),
]

def getNameAndCano (m):
    try:
        name = m.name()
    except IndigoException, e:
        name = getIndigoExceptionText(e)
    try:
        cansm = m.canonicalSmiles()
    except IndigoException, e:
        cansm = getIndigoExceptionText(e)
    try:
        sm = m.smiles()
    except IndigoException, e:
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
        except IndigoException, e:
            pass
        idx += 1
