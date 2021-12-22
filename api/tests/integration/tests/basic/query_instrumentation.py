import errno
import sys

sys.path.append("../../common")
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")

if not os.path.exists(joinPathPy("out", __file__)):
    try:
        os.makedirs(joinPathPy("out", __file__))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

t = indigo.loadMolecule("CC1=NC(C)=C(C)C(C)=N1")


def testMatch(t, q):
    matcher = indigo.substructureMatcher(t)
    msg = ""
    if matcher.match(q) != None:
        msg = "True"
    else:
        msg = "False"
    print("Match: %s. Count = %d" % (msg, matcher.countMatches(q)))


print("****** 1 ********")
q = indigo.createQueryMolecule()
a1 = q.addAtom("C")
testMatch(t, q)
a1.resetAtom("c")
testMatch(t, q)
a1.resetAtom("[#6]")
testMatch(t, q)

a2 = q.addAtom("N")
testMatch(t, q)
a2.resetAtom("c")
a2.addBond(a1, 1)
testMatch(t, q)

print("****** 2 ********")
indigo.setOption("embedding-uniqueness", "none")
q = indigo.loadSmarts("c1c[nH]c[nH]c1")
testMatch(t, q)
for a in q.iterateAtoms():
    a.removeConstraints("hydrogens")
testMatch(t, q)

a1 = q.getAtom(1)
a2 = q.getAtom(2)
a3 = q.getAtom(3)
print(a1.symbol())
print(a2.symbol())

for a in q.iterateAtoms():
    a.removeConstraints("atomic-number")

testMatch(t, q)

a1.addConstraint("substituents", "3")
testMatch(t, q)

a3.addConstraint("substituents", "3")
testMatch(t, q)

a2.addConstraint("substituents", "2")
testMatch(t, q)

print("****** 3 ********")
q = indigo.createQueryMolecule()
a1 = q.addAtom("C")
testMatch(t, q)
a1.addConstraintOr("smarts", "n")
testMatch(t, q)
a1.addConstraint("aromaticity", "aromatic")
testMatch(t, q)
a1.removeConstraints("aromaticity")
testMatch(t, q)
a1.addConstraint("aromaticity", "aromatic")
testMatch(t, q)
a1.removeConstraints("aromaticity")
testMatch(t, q)
a1.addConstraint("aromaticity", "aliphatic")
testMatch(t, q)

constrants = [
    ("atomic-number", "2"),
    ("charge", "2"),
    ("isotope", "2"),
    ("radical", "2"),
    ("valence", "2"),
    ("connectivity", "2"),
    ("total-bond-order", "2"),
    ("hydrogens", "2"),
    ("substituents", "2"),
    ("ring", "2"),
    ("smallest-ring-size", "2"),
    ("ring-bonds", "2"),
    ("rsite-mask", "2"),
    ("rsite", "2"),
    ("aromaticity", "aliphatic"),
]

for c, v in constrants:
    a1.addConstraint(c, v)
print(q.smiles())
testMatch(t, q)

for c, v in constrants:
    a1.addConstraintNot(c, v)
for c, v in constrants:
    a1.addConstraintOr(c, v)
for c, v in constrants:
    a1.removeConstraints(c)
for c, v in constrants:
    a1.addConstraint(c, v)
print(q.smiles())
testMatch(t, q)

a1.resetAtom("")
testMatch(t, q)
a1.addConstraintNot("atomic-number", "1")
testMatch(t, q)

print("****** 2: highlighting ********")
m = indigo.loadMolecule("NCCNCCN")
print(m.smiles())
matcher = indigo.substructureMatcher(m)
q = indigo.loadQueryMolecule("NCC")
print(m.smiles())
print("Count: %d" % (matcher.countMatches(q)))
a = m.getAtom(3)
a.highlight()
matcher = indigo.substructureMatcher(m)
print(m.smiles())
print("Count: %d" % (matcher.countMatches(q)))
q.getAtom(0).addConstraint("highlighting", "true")
print("Count: %d" % (matcher.countMatches(q)))
