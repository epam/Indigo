import os
import sys
import errno

sys.path.append('../../common')
from env_indigo import *

if not os.path.exists(joinPath("out")):
    try:
        os.makedirs(joinPath("out"))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

indigo = Indigo()

def checkCisTransLayout (m):
	cs1 = m.canonicalSmiles()
	print(cs1)

	m.layout()

	m2 = indigo.loadMolecule(m.molfile())
	cs2 = m2.canonicalSmiles()
	print(cs2)
	if cs1 != cs2:
		sys.stderr.write("Error: canonical SMIILES do not match\n")

	m2.layout()
	m3 = indigo.loadMolecule(m2.molfile())
	cs3 = m3.canonicalSmiles()
	print(cs3)
	if cs1 != cs3:
		sys.stderr.write("Error: canonical SMIILES do not match\n")


print("Cis-trans check for inverse")
m = indigo.loadMoleculeFromFile(joinPath("molecules", "cid-62301-changed.mol"))
checkCisTransLayout(m)

print("Cis-trans check for bad angle")
m = indigo.loadMoleculeFromFile(joinPath("molecules", "cid-1006416-changed.mol"))
checkCisTransLayout(m)

