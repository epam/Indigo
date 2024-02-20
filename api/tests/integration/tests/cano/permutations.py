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
        joinPathPy("../../../../../data/molecules/basic/helma.smi", __file__),
        indigo.iterateSmilesFile,
    ),
    (
        joinPathPy("molecules/cis_trans.smi", __file__),
        indigo.iterateSmilesFile,
    ),
    (
        joinPathPy(
            "../../../../../data/molecules/stereo/stereo_cis_trans.sdf",
            __file__,
        ),
        indigo.iterateSDFile,
    ),
    (
        joinPathPy("molecules/complicated_cis_trans.sdf", __file__),
        indigo.iterateSDFile,
    ),
    (
        joinPathPy("molecules/complicated_cis_trans_h.sdf", __file__),
        indigo.iterateSDFile,
    ),
    (
        joinPathPy("molecules/test_explicit_h.smi", __file__),
        indigo.iterateSmilesFile,
    ),
    (
        joinPathPy("molecules/arom-ambiguous.smi", __file__),
        indigo.iterateSmilesFile,
    ),
    (
        joinPathPy("molecules/arom-ambiguous.sdf", __file__),
        indigo.iterateSDFile,
    ),
]


def random_permutation(iterable, r=None):
    """Random selection from itertools.permutations(iterable, r)"""
    pool = tuple(iterable)
    if r is None:
        r = len(pool)
    return list(random.sample(pool, r))


def testMol(mol):
    mol.clearXYZ()
    mol_for_test = mol.clone()
    base_smiles = mol_for_test.canonicalSmiles()
    print(base_smiles)
    mol_for_test.foldHydrogens()
    base_smiles2 = mol_for_test.canonicalSmiles()
    if base_smiles != base_smiles2:
        msg = (
            "  Canonical smiles has changes after fold hydrogens: %s -> %s"
            % (
                base_smiles,
                base_smiles2,
            )
        )
        print(msg)
        sys.stderr.write(msg + "\n")

    try:
        mol_for_test.unfoldHydrogens()
    except IndigoException as e:
        print("unfoldHydrogens: %s" % (getIndigoExceptionText(e)))
    base_smiles3 = mol_for_test.canonicalSmiles()
    if base_smiles != base_smiles3:
        msg = (
            "  Canonical smiles has changes after fold->unfold hydrogens: %s -> %s"
            % (base_smiles, base_smiles3)
        )
        print(msg)
        sys.stderr.write(msg + "\n")
    mol_for_test.foldHydrogens()
    base_smiles2 = mol_for_test.canonicalSmiles()
    if base_smiles != base_smiles2:
        msg = (
            "  Canonical smiles has changes after fold->unfold->fold hydrogens: %s -> %s"
            % (base_smiles, base_smiles2)
        )
        print(msg)
        sys.stderr.write(msg + "\n")

    # collect atom indices
    indices = [x.index() for x in mol.iterateAtoms()]

    # test some of permutations
    ncount = 20
    for it in range(ncount):
        perm = random_permutation(indices, len(indices))
        # perm_mol = mol.createSubmolecule(perm)
        perm_mol = mol.clone()

        need_fold = random.choice([True, False])
        need_unfold = random.choice([True, False])
        if need_fold:
            perm_mol.foldHydrogens()
        if need_unfold:
            try:
                perm_mol.unfoldHydrogens()
            except IndigoException as e:
                pass

        need_fold2 = random.choice([True, False])
        need_unfold2 = random.choice([True, False])
        if need_fold2:
            perm_mol.foldHydrogens()
        if need_unfold2:
            try:
                perm_mol.unfoldHydrogens()
            except IndigoException as e:
                pass

        perm_cano_sm = perm_mol.canonicalSmiles()
        if perm_cano_sm != base_smiles:
            msg = "  %d, %d, %d, %d: %s -> %s" % (
                need_fold,
                need_unfold,
                need_fold2,
                need_unfold2,
                perm,
                perm_cano_sm,
            )
            print(msg)
            sys.stderr.write(msg + "\n")
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
