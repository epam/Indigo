import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")
print("****** Query reload ********")
q = indigo.loadQueryMoleculeFromFile(joinPath("molecules/q_atom_list.mol"))
qmf1 = q.molfile()
print(qmf1)
q2 = indigo.loadQueryMolecule(q.molfile())
qmf2 = q2.molfile()
if qmf1 != qmf2:
   print("Error: reloaded query is different:\n%s\n" % (q2.molfile()))
# Check that queires are equivalent
matcher = indigo.substructureMatcher(indigo.loadMolecule("[Sc]CN[He]"))
none1 = matcher.match(q)
none2 = matcher.match(q2)
if none1 or none2:
    print("Error: matching results are not None: %s and %s" % (none1, none2))

print("****** Remove constraints and reload ********")
q = indigo.loadQueryMolecule("c1[nH]c2c(c(N)[n+]([O-])c[n]2)[n]1")
t = indigo.loadMolecule("c1[n]c2c(N)[n+]([O-])c[n]c2[n]1[C@H]1[C@@H](O)[C@H](O)[C@H](CO)O1")

original_smiles = q.smiles()
print(q.smiles())

matcher = indigo.substructureMatcher(t)
has_match_orig = matcher.match(q) != None
print(has_match_orig)
for a in q.iterateAtoms():
    a.removeConstraints("hydrogens")
print(q.smiles())
has_match = (matcher.match(q) != None)
print(has_match)

q2 = q.clone()
print(q2.smiles())
has_match2 = (matcher.match(q2) != None)
print(has_match2)
if has_match != has_match2:
    sys.stderr.write("Error: query molecule match is different after cloning\n")

# reload query from original smiles
q3 = indigo.loadQueryMolecule(original_smiles)
print(q3.smiles())
has_match3 = (matcher.match(q3) != None)
print(has_match3)
if has_match3 != has_match_orig:
    sys.stderr.write("Error: query molecule match is different after reloading from SMILES\n")

print("****** Bad valence, smiles and unfold ********")
m = indigo.loadMolecule("C\C=C(/N(O)=O)N(O)=O")    
sm = m.smiles()
print(m.smiles())
print(m.canonicalSmiles())
try:
    m.unfoldHydrogens()
except IndigoException, e:
    print("%s" % (getIndigoExceptionText(e)))
    
# If there was an exception in unfoldHydrogens then molecule should not be changed
sm2 = m.smiles()
if sm2 != sm:
    sys.stderr.write("Error: %s != %s" % (sm, sm2))

