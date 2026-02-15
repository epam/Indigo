import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import (  # noqa: F401
    Bingo,
    BingoException,
    BingoObject,
    Indigo,
    IndigoException,
    IndigoInchi,
    IndigoObject,
    IndigoRenderer,
    dataPath,
    dir_exists,
    file_exists,
    file_size,
    getIndigoExceptionText,
    joinPathPy,
    makedirs,
    relativePath,
    rmdir,
    threading,
)

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
