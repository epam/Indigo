import random
import sys

sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("treat-x-as-pseudoatom", "1")
indigo.setOption("ignore-stereochemistry-errors", "1")
mol_db_names = [
    joinPath("../../../../../data/molecules/basic/zinc-slice.sdf.gz"),
    joinPath("../../../../../data/molecules/basic/thiazolidines.sdf"),
    joinPath("../../../../../data/molecules/basic/sugars.sdf")]


def random_permutation(iterable, r=None):
    """Random selection from itertools.permutations(iterable, r)"""
    pool = tuple(iterable)
    if r is None:
        r = len(pool)
    return list(random.sample(pool, r))


def testMol(mol):
    for bond in mol.iterateBonds():
        if bond.topology() == indigo.RING and bond.bondOrder() == 2:
            bond.resetStereo()
    mol.resetSymmetricCisTrans()
    base_smiles = mol.canonicalSmiles()
    perm_mol = indigo.loadMolecule(base_smiles)
    perm_smiles = perm_mol.canonicalSmiles()
    if perm_smiles != base_smiles:
        msg = "  Smiles: %s after reload from smiles %s. Permuted canonical smiles: %s\n" % (
        mol.name(), base_smiles, perm_smiles)
        print(msg)
        sys.stderr.write(msg + "\n")
    if not indigo.exactMatch(mol, perm_mol):
        msg = "  Exact: %s after reload from smiles %s. Permuted canonical smiles: %s\n" % (
        mol.name(), base_smiles, perm_smiles)
        print(msg)
        sys.stderr.write(msg + "\n")


for db_name in mol_db_names:
    print("Database: %s" % relativePath(db_name))
    idx = 0
    for item in indigo.iterateSDFile(db_name):
        # print("%s (#%s)" % (item.name(), idx))
        if item.name() == "":
            item.setName("#%d" % idx)
        try:
            testMol(item)
        except IndigoException as e:
            print("Error: %s" % (getIndigoExceptionText(e)))
        idx += 1
