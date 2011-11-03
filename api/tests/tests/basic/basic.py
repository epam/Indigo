import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")
print("****** Query reload ********")
q = indigo.loadQueryMoleculeFromFile("molecules/q_atom_list.mol")
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
