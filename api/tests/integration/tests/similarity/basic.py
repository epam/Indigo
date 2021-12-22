import sys

sys.path.append("../../common")
from env_indigo import *

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", "1")
indigo.setOption("ignore-noncritical-query-features", "true")

print("*** Self-similarity ***")


def checkOne(mol, metric):
    v = indigo.similarity(mol, mol, metric)
    print("  %s: %s" % (metric, v))
    if v != 1.0:
        sys.stderr.write(
            "    mol %s, metric %s, sim = %s but must be 1\n"
            % (mol.name(), metric, v)
        )


for m in indigo.iterateSDFile(
    joinPathPy("molecules/thiazolidines_slice.sdf", __file__)
):
    print("%s: %s" % (m.name(), m.canonicalSmiles()))
    checkOne(m, "tversky")
    checkOne(m, "tversky 1 0")
    checkOne(m, "tversky 0 1")
    checkOne(m, "tversky 0.7 0.3")
    checkOne(m, None)
    checkOne(m, "euclid-sub")

print("*** Test 01 ***")
m1 = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/ind-452-a1.mol", __file__)
)
m2 = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/ind-452-a2.mol", __file__)
)
print("m1: %s" % (m1.canonicalSmiles()))
print("m1: %s" % (m2.canonicalSmiles()))
print("Similarity: %0.4f" % (indigo.similarity(m1, m2, "tanimoto")))

print("*** Test for edit distance ***")
mols = [
    indigo.loadMolecule("C(COCC1=CC=CC=C1)CC1=CC=CC=C1"),
    indigo.loadMolecule("C(CNCC1=CC=CC=C1)CC1=CC=CC=C1"),
    indigo.loadMolecule("C(CNCC1=CC=CC=C1)CC1-CC=CC=C1"),
    indigo.loadMolecule("C(CNCC1=CC=CC=C1)CC1-CC=SC=C1"),
    indigo.loadMolecule("C(CNCC1=CC-CC=C1)CC1-CC=SC=C1"),
    indigo.loadMolecule("CCCC1CS=CC=C1"),
    indigo.loadMolecule("C1CC=SC=C1"),
]


def getMatrix(mols, metric):
    print(metric)
    for idx, m1 in enumerate(mols):
        print("%d: %s" % (idx + 1, m1.smiles()))
    for m1 in mols:
        for m2 in mols:
            sim = indigo.similarity(m1, m2, metric)
            sys.stdout.write(" %0.3f" % (round(sim, 3)))
        sys.stdout.write("\n")


metrics = ["tanimoto", "tversky", "normalized-edit"]
for metric in metrics:
    getMatrix(mols, metric)

print("*** Test for single atom fingerprints ***")
mols_str = ["[H][H]", "C", "N", "CN", "CCN", "CCNN", "FOF"]
mols = [indigo.loadMolecule(m) for m in mols_str]

metrics = ["tanimoto", "tversky", "normalized-edit"]
for metric in metrics:
    getMatrix(mols, metric)
