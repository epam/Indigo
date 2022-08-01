import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "1")
indigo.setOption("ignore-noncritical-query-features", "true")

print("****** Query reload ********")
q = indigo.loadQueryMoleculeFromFile(
    joinPathPy("molecules/q_atom_list.mol", __file__)
)
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
t = indigo.loadMolecule(
    "c1[n]c2c(N)[n+]([O-])c[n]c2[n]1[C@H]1[C@@H](O)[C@H](O)[C@H](CO)O1"
)

original_smiles = q.smiles()
print(q.smiles())

matcher = indigo.substructureMatcher(t)
has_match_orig = matcher.match(q) != None
print(has_match_orig)
for a in q.iterateAtoms():
    a.removeConstraints("hydrogens")
print(q.smiles())
has_match = matcher.match(q) != None
print(has_match)

q2 = q.clone()
print(q2.smiles())
has_match2 = matcher.match(q2) != None
print(has_match2)
if has_match != has_match2:
    sys.stderr.write(
        "Error: query molecule match is different after cloning\n"
    )

# reload query from original smiles
q3 = indigo.loadQueryMolecule(original_smiles)
print(q3.smiles())
has_match3 = matcher.match(q3) != None
print(has_match3)
if has_match3 != has_match_orig:
    sys.stderr.write(
        "Error: query molecule match is different after reloading from SMILES\n"
    )

print("****** Bad valence, smiles and unfold ********")
m = indigo.loadMolecule("C\C=C(/N(O)=O)N(O)=O")
sm = m.smiles()
print(m.smiles())
print(m.canonicalSmiles())
try:
    m.unfoldHydrogens()
except IndigoException as e:
    print("%s" % (getIndigoExceptionText(e)))

# If there was an exception in unfoldHydrogens then molecule should not be changed
sm2 = m.smiles()
if sm2 != sm:
    sys.stderr.write("Error: %s != %s" % (sm, sm2))

print("****** Serialize and atom changing ********")
m = indigo.loadMolecule("CC[C@@H](N)\C=C/C")
print(m.smiles())
print(m.canonicalSmiles())

for a in m.iterateAtoms():
    a.resetAtom("*")

print(m.smiles())
print(m.canonicalSmiles())

try:
    m2 = indigo.unserialize(m.serialize())
    print(m2.smiles())
    print(m2.canonicalSmiles())
except IndigoException as e:
    sys.stderr.write("Error: %s" % (getIndigoExceptionText(e)))

print("****** Anormal properties ********")
m = indigo.loadMolecule("[WH7][W][W][W+10][W][W-10]")
for a in m.iterateAtoms():
    print("%d %d" % (a.charge(), a.valence()))

try:
    m2 = indigo.unserialize(m.serialize())
    print(m2.smiles())
    print(m2.canonicalSmiles())
    for a in m2.iterateAtoms():
        print("%d %d" % (a.charge(), a.valence()))
except IndigoException as e:
    sys.stderr.write("Error: %s" % (getIndigoExceptionText(e)))

print("****** Unmarked stereobonds ********")
m = indigo.loadMoleculeFromFile(joinPathPy("molecules/stereo.mol", __file__))
print(m.canonicalSmiles())
m.clearStereocenters()
print(m.canonicalSmiles())

m2 = indigo.loadMolecule(m.molfile())
print(m2.canonicalSmiles())
if m.canonicalSmiles() != m2.canonicalSmiles():
    sys.stderr.write("Error: canonical smiles are different")

print("****** Chemical formula ********")
print(indigo.loadMolecule("[Br][I]").grossFormula())
print(indigo.loadMolecule("[Br][H]").grossFormula())
print(indigo.loadMolecule("OS(=O)(=O)O").grossFormula())
print(indigo.loadMolecule("CI").grossFormula())
print(indigo.loadMolecule("CCBr").grossFormula())
print(indigo.loadMolecule("[H]O[H]").grossFormula())
print(indigo.loadMolecule("c1ccccc1").grossFormula())
print(indigo.loadMolecule("c1ccccc1[He]").grossFormula())
print(indigo.loadMolecule("c1ccccc1[He][Br]").grossFormula())


print("****** Nei iterator ********")
m = indigo.loadMolecule("CCC1=CC2=C(C=C1)C(CC)=CC(CC)=C2")
for v in m.iterateAtoms():
    print("v:%d" % (v.index()))
    for nei in v.iterateNeighbors():
        print(
            "  neighbor atom %d is connected by bond %d"
            % (nei.index(), nei.bond().index())
        )

print("****** Structure normalization ********")
try:
    m = indigo.loadMolecule("[H]N(C)C(\\[H])=C(\\[NH2+][O-])N(=O)=O")
    print(m.smiles())
    print(m.normalize(""))
    print(m.smiles())
    print(m.normalize(""))
    print(m.smiles())
except IndigoException as e:
    print("Exception: {0}".format(getIndigoExceptionText(e)))

print("****** R-group big index ********")
mols = ["molecules/r31.mol", "molecules/r32.mol", "molecules/r128.mol"]
for molfile in mols:
    for loader, type in [
        (indigo.loadMoleculeFromFile, "molecule"),
        (indigo.loadQueryMoleculeFromFile, "query"),
    ]:
        print(molfile + " " + type + ":")
        try:
            m = loader(joinPathPy(molfile, __file__))
            m.molfile()  # check molfile generation
            print("  " + m.smiles())
        except IndigoException as e:
            print("  Error: %s" % (getIndigoExceptionText(e)))

print("****** Smiles with R-group ********")
smiles_set = ["NC[*][*][*][*]", "[*][*]NC[*][*]", "[*][*][*][*]NC"]
for smiles in smiles_set:
    print("Smiles: " + smiles)
    m = indigo.loadMolecule(smiles)
    print("  Smiles:      " + m.smiles())
    print("  Cano smiles: " + m.canonicalSmiles())
    for a in m.iterateAtoms():
        print("  %d: %s" % (a.index(), a.symbol()))

print("****** Smiles <-> Molfile ********")
m = indigo.loadQueryMolecule("[CH6]")
print(m.smiles())
for val in ["2000", "3000", "auto"]:
    print(" molfile-saving-mode: %s" % (val))
    indigo.setOption("molfile-saving-mode", val)

    m2 = indigo.loadMolecule(m.molfile())
    print(m2.smiles())

    m3 = indigo.loadQueryMolecule(m.molfile())
    print(m3.smiles())

print("****** SMARTS and query SMILES ********")
q = indigo.loadSmarts("[#8;A]-[*]-[#6;A](-[#9])(-[#9])-[#9]")
print(q.smiles())
q2 = indigo.loadQueryMolecule(q.smiles())
print(q2.smiles())

q3 = indigo.loadQueryMolecule("******** |$;AH_p;Q_e;QH_p;M_p;MH_p;X_p;XH_p$|")
print(q3.smiles())
print(q3.molfile())

print("****** Large symmetric molecule ********")
m = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/large-symmetric.smi", __file__)
)
print(m.smiles())
m = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/large-symmetric.mol", __file__)
)
print(m.smiles())


print("****** Symmetric stereocenters and cis-trans bonds ********")
m = indigo.loadMolecule("C[C@H]1CCC(CC1)C(\C1CC[C@H](C)CC1)=C(\C)C1CCCCC1")
print(m.smiles())
m2 = m.clone()
m.resetSymmetricCisTrans()
print(m.smiles())
m2.resetSymmetricStereocenters()
print(m2.smiles())
m.resetSymmetricStereocenters()
print(m.smiles())

print("****** Remove bonds ********")
m = indigo.loadMolecule("CNCNCNCN")
print(m.smiles())
m.removeBonds([1, 3, 4])
print(m.smiles())

print(
    "****** Overlapping stereocenters due to hydrogens folding bug fix check *****"
)
m = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/pubchem-150858.mol", __file__)
)
cs = m.canonicalSmiles()
print(cs)
m.foldHydrogens()
m2 = indigo.loadMolecule(m.molfile())
cs2 = m2.canonicalSmiles()
print(cs2)
if cs != cs2:
    print("Bug!")

# another bug check for missing markSterebonds call in the foldHydrogens method
m.markStereobonds()
m3 = indigo.loadMolecule(m.molfile())
cs3 = m3.canonicalSmiles()
print(cs3)
if cs != cs3:
    print("Bug!")

print("****** SMILES cis-trans check *****")
m = indigo.loadMoleculeFromFile(
    joinPathPy("molecules/016_26-large.mol", __file__)
)
print(m.smiles())
print(m.canonicalSmiles())

print("****** Empty SDF saver *****")
buffer = indigo.writeBuffer()
sdfSaver = indigo.createSaver(buffer, "sdf")
sdfSaver.close()
print(len(buffer.toBuffer()))
print(len(buffer.toString()))

print("****** Normalize and serialize *****")
mols = [
    "[O-]/[N+](=C(/[H])\\C1C([H])=C([H])C([H])=C([H])C=1[H])/C1C([H])=C([H])C([H])=C([H])C=1[H]",
    "C\\C=C\\C1=CC=CC(\\C=[N+](/[O-])C2=C(\\C=C\\C)C=CC=C2)=C1",
]
for mstr in mols:
    m = indigo.loadMolecule(mstr)
    print(m.canonicalSmiles())
    m.normalize()
    m2 = indigo.unserialize(m.serialize())
    print(m2.canonicalSmiles())

print("***** Serialization of aromatic hydrogens *****")
m = indigo.loadMolecule("C[c]1(C)ccccc1")
q = indigo.loadQueryMolecule("N([H])[H]")
m2 = indigo.unserialize(m.serialize())
matcher = indigo.substructureMatcher(m2)
assert matcher.match(q) == None

print("***** Reset options check *****")
# Create molecules and set their names
m1 = indigo.loadMolecule("[H][C@](C)(N)O")
m1.setName("Molecule 1")
m2 = indigo.loadMolecule("C1=CC=CC=C1")
m2.setName("Molecule 2")

indigo.setOption("smiles-saving-write-name", True)

# Create string stream and save molecules in SMILES format into it
buffer = indigo.writeBuffer()
saver = indigo.createSaver(buffer, "smi")
saver.append(m1)
saver.append(m2)
print(buffer.toString())

indigo.resetOptions()

# Create string stream and save molecules in SMILES format into it
buffer = indigo.writeBuffer()
saver = indigo.createSaver(buffer, "smi")
saver.append(m1)
saver.append(m2)
print(buffer.toString())
