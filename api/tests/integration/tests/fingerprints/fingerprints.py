import sys

sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("treat-x-as-pseudoatom", "1")
mol_db_names = [
    (joinPath("../../../../../data/molecules/basic/zinc-slice.sdf.gz"), indigo.iterateSDFile),
    (joinPath("../../../../../data/molecules/basic/thiazolidines.sdf"), indigo.iterateSDFile),
    (joinPath("../../../../../data/molecules/basic/sugars.sdf"), indigo.iterateSDFile)
]


def process(m):
    fp = m.fingerprint("full")
    print("  " + fp.toString())


for db_name, load_fund in mol_db_names:
    print("Database: %s" % relativePath(db_name))
    idx = 1
    for item in load_fund(db_name):
        print("#%d:" % (idx))
        # Check serialization
        try:
            process(item)
        except IndigoException as e:
            print("  Exception: %s" % (getIndigoExceptionText(e)))
        idx += 1
