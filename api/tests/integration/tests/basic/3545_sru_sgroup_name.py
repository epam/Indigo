import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import Indigo

indigo = Indigo()

print(
    "****** Issue #3545: setSGroupName and getSGroupName for SRU sgroups ********"
)

mol = indigo.createMolecule()
mol.addAtom("C")

query = indigo.createQueryMolecule()
query.addAtom("C")

matcher = indigo.substructureMatcher(mol)
match = matcher.match(query)

if match:
    # 1. Create SRU SGroup
    sru = mol.createSGroup("SRU", match, "TEST_SRU")
    # 2. Test get name
    name_before = sru.getSGroupName()
    print("SRU name before: {}".format(name_before))

    # 3. Test set name
    sru.setSGroupName("NEW_TEST_SRU")
    name_after = sru.getSGroupName()
    print("SRU name after: {}".format(name_after))
else:
    print("ERROR: match not found")
