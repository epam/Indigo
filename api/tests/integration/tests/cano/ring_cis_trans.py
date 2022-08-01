import os
import random
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("treat-x-as-pseudoatom", "1")
indigo.setOption("ignore-noncritical-query-features", "true")

mol_db_names = [
    (
        joinPathPy(
            "../../../../../data/molecules/stereo/ring-cis-trans.sdf", __file__
        ),
        indigo.iterateSDFile,
    )
]


def random_permutation(iterable, r=None):
    """Random selection from itertools.permutations(iterable, r)"""
    pool = tuple(iterable)
    if r is None:
        r = len(pool)
    return list(random.sample(pool, r))


def testMol(mol):
    base_smiles = mol.canonicalSmiles()
    print(base_smiles)

    # collect atom indices
    indices = [x.index() for x in mol.iterateAtoms()]

    # test some of permutations
    for it in range(10):
        perm = random_permutation(indices, len(indices))
        perm_mol = mol.createSubmolecule(perm)
        perm_cano_sm = perm_mol.canonicalSmiles()
        if perm_cano_sm != base_smiles:
            print("  %s -> %s" % (perm, perm_cano_sm))
    return base_smiles


for db_name, load_fund in mol_db_names:
    print("Database: %s" % relativePath(db_name))
    idx = 0
    for item in load_fund(db_name):
        try:
            name = item.name()
        except IndigoException as e:
            name = getIndigoExceptionText(e)
        print("%s (#%s)" % (name, idx))
        try:
            testMol(item)
        except IndigoException as e:
            print("Error: %s" % (getIndigoExceptionText(e)))
        idx += 1
