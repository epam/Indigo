import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import Indigo  # noqa

indigo = Indigo()

print("****** Issue #3544: SGroup indexing with unfolded hydrogens ********")

# Test 1: Query with unfoldHydrogens, SUP sgroup
print("Test 1: SUP sgroup from query with unfolded hydrogens")
mol = indigo.createMolecule()
mol.addAtom("C")

query = indigo.createQueryMolecule()
query.addAtom("C")
query.unfoldHydrogens()

matcher = indigo.substructureMatcher(mol)
match = matcher.match(query)

if match:
    sgroup = mol.createSGroup("SUP", match, "TEST")
    atoms = []
    for atom in sgroup.iterateAtoms():
        atoms.append(atom.symbol())
    print("SGroup atoms: %s" % " ".join(atoms))
else:
    print("ERROR: match not found")

# Test 2: Both target and query with unfoldHydrogens
print("Test 2: SUP sgroup, both target and query with unfolded hydrogens")
mol2 = indigo.createMolecule()
mol2.addAtom("C")
mol2.unfoldHydrogens()

query2 = indigo.createQueryMolecule()
query2.addAtom("C")
query2.unfoldHydrogens()

matcher2 = indigo.substructureMatcher(mol2)
match2 = matcher2.match(query2)

if match2:
    sgroup2 = mol2.createSGroup("SUP", match2, "TEST2")
    atoms2 = []
    for atom in sgroup2.iterateAtoms():
        atoms2.append(atom.symbol())
    print("SGroup atoms: %s" % " ".join(sorted(atoms2)))
else:
    print("ERROR: match not found")

# Test 3: DAT sgroup type with unfoldHydrogens
print("Test 3: DAT sgroup from query with unfolded hydrogens")
mol3 = indigo.createMolecule()
mol3.addAtom("C")

query3 = indigo.createQueryMolecule()
query3.addAtom("C")
query3.unfoldHydrogens()

matcher3 = indigo.substructureMatcher(mol3)
match3 = matcher3.match(query3)

if match3:
    sgroup3 = mol3.createSGroup("DAT", match3, "TEST3")
    atoms3 = []
    for atom in sgroup3.iterateAtoms():
        atoms3.append(atom.symbol())
    print("SGroup atoms: %s" % " ".join(atoms3))
else:
    print("ERROR: match not found")

# Test 4: Control case - no unfoldHydrogens (should work regardless)
print("Test 4: Control - SUP sgroup without unfolded hydrogens")
mol4 = indigo.createMolecule()
mol4.addAtom("C")

query4 = indigo.createQueryMolecule()
query4.addAtom("C")

matcher4 = indigo.substructureMatcher(mol4)
match4 = matcher4.match(query4)

if match4:
    sgroup4 = mol4.createSGroup("SUP", match4, "TEST4")
    atoms4 = []
    for atom in sgroup4.iterateAtoms():
        atoms4.append(atom.symbol())
    print("SGroup atoms: %s" % " ".join(atoms4))
else:
    print("ERROR: match not found")

# Test 5: Multi-atom query with unfoldHydrogens in a larger molecule
print("Test 5: SUP sgroup from multi-atom query match with unfolded hydrogens")
mol5 = indigo.loadMolecule("CCCC")

query5 = indigo.loadQueryMolecule("[#6][#6]")
query5.unfoldHydrogens()

matcher5 = indigo.substructureMatcher(mol5)
match5 = matcher5.match(query5)

if match5:
    sgroup5 = mol5.createSGroup("SUP", match5, "TEST5")
    atoms5 = []
    for atom in sgroup5.iterateAtoms():
        atoms5.append(atom.symbol())
    print("SGroup atoms: %s" % " ".join(atoms5))
else:
    print("ERROR: match not found")
