import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()
indigo2 = Indigo()
mol1 = indigo.loadMolecule("C mol1")
mol2 = indigo2.loadMolecule("N mol2")
c1 = mol1.canonicalSmiles()
c2 = mol2.canonicalSmiles()
print(c1)
print(c2)
if c1 != "C":
    sys.stderr.write("%s != C\n" % c1)
if c2 != "N":
    sys.stderr.write("%s != N\n" % c1)
for m1, m2 in zip(
    indigo.iterateSmilesFile(joinPathPy("molecules/helma.smi", __file__)),
    indigo2.iterateSmilesFile(joinPathPy("molecules/helma.smi", __file__)),
):
    try:
        c1 = m1.canonicalSmiles()
    except IndigoException as e:
        c1 = getIndigoExceptionText(e)

    try:
        c2 = m2.canonicalSmiles()
    except IndigoException as e:
        c2 = getIndigoExceptionText(e)
    print("%s, %s" % (c1, c2))
    if c1 != c2:
        sys.stderr.write("Error: %s != %s" % (c1, c2))
