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


def iterateLoopIterator(it, output):
    cnt = 0
    rings = set()
    for loop in it:
        vset = tuple([v.index() for v in loop.iterateAtoms()])
        if vset in rings:
            sys.stderr.write(
                "Ring %s has already been enumerated\n" % (str(vset))
            )
        rings.add(vset)
        output.write("    %s\n" % (str(vset)))
        cnt += 1
    return cnt


def iterateLoops(m, output):
    output.write("  Rings 3, 10:\n")
    cnt = iterateLoopIterator(m.iterateRings(3, 10), output)
    output.write("  Total rings: %d\n" % (cnt))

    output.write("  Rings 4, 8:\n")
    cnt2 = iterateLoopIterator(m.iterateRings(4, 8), output)
    output.write("  Total rings: %d\n" % (cnt2))

    output.write("  SSSR:\n")
    sssrcnt = iterateLoopIterator(m.iterateSSSR(), output)
    output.write("  Total SSSR rings: %d\n" % (sssrcnt))

    return cnt, sssrcnt, cnt2


def iterateLoopsSmiles(smiles, output):
    print("Testing: " + smiles)
    return iterateLoops(indigo.loadMolecule(smiles), output)


def random_permutation(iterable, r=None):
    """Random selection from itertools.permutations(iterable, r)"""
    pool = tuple(iterable)
    if r is None:
        r = len(pool)
    return list(random.sample(pool, r))


def getPermutatedMolecule(m):
    indices = [x.index() for x in m.iterateAtoms()]
    perm = random_permutation(indices, len(indices))
    perm_mol = m.createSubmolecule(perm)
    return perm_mol, perm


cnt1 = iterateLoopsSmiles("c1c2CC(C)Cc2ccc1", sys.stdout)
cnt2 = iterateLoopsSmiles("c1cccc2c1CC(C2)C", sys.stdout)
if cnt1 != cnt2:
    sys.stderr.write("Loop count is different: %s and %s" % (cnt1, cnt2))


class NullOuput:
    def write(self, s):
        pass


print("Testing set of molecules")
idx = 1
for item in indigo.iterateSDFile(joinPathPy("molecules/rings.sdf", __file__)):
    print("#%d: " % (idx))
    rings = iterateLoops(item, sys.stdout)
    for pc in range(5):
        permmol, perm = getPermutatedMolecule(item)
        rings2 = iterateLoops(permmol, NullOuput())
        if rings != rings2:
            sys.stderr.write(
                "Loop count is different: %s and %s\n" % (rings, rings2)
            )
            sys.stderr.write("Permutation: %s\n" % (str(perm)))
    idx += 1
