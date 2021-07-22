import sys

sys.path.append('../../common')
from env_indigo import Indigo, IndigoException, joinPath, getIndigoExceptionText

indigo = Indigo()


def round(x):
    if abs(x) < 1e-3:
        return 0
    return x


def printCoords(m):
    for atom in m.iterateAtoms():
        (x, y, z) = atom.xyz()
        x = round(x)
        y = round(y)
        z = round(z)
        print("%0.3f %0.3f %0.3f" % (x, y, z))


def test1():
    print("Test1:")
    reaction = indigo.loadQueryReaction(
        "[N-,P-,As-,Sb-,O-,S-,Se-,Te-:1][C:2]=[N+,P+,As+,Sb+,O+,S+,Se+,Te+:3]>>[N,P,As,Sb,O,S,Se,Te:1]=[C:2][N,P,As,Sb,O,S,Se,Te:3]")
    molecule = indigo.loadMoleculeFromFile(joinPath("tests_transform_basic/1/mon.mol"))
    print(molecule.canonicalSmiles())
    molFromSMILES = indigo.loadMolecule(molecule.canonicalSmiles())
    molFromSMILES.markEitherCisTrans()
    indigo.transform(reaction, molFromSMILES)
    print(molFromSMILES.canonicalSmiles())


def test2():
    print("Test2:")
    reaction = indigo.loadQueryReaction("[S:1]>>[P:1]I")
    molecule1 = indigo.loadMoleculeFromFile(joinPath("tests_transform_basic/2/mon1.mol"))
    molecule2 = indigo.loadMoleculeFromFile(joinPath("tests_transform_basic/2/mon2.mol"))
    indigo.transform(reaction, molecule1)
    indigo.transform(reaction, molecule2)
    print(molecule1.smiles())
    print(molecule2.smiles())


def testInd194():
    print("Test IND-194:")
    reaction = indigo.loadQueryReactionFromFile(joinPath("tests_transform_basic/ind_194/rxn.rxn"))
    molecule1 = indigo.loadMoleculeFromFile(joinPath("tests_transform_basic/ind_194/mol.mol"))
    print(molecule1.smiles())
    indigo.transform(reaction, molecule1)
    print(molecule1.smiles())


def testInd661():
    print("Test IND-194:")
    rxn_without_aam = indigo.loadQueryReaction("[C][S+]([C])[O-]>>[C]S([C])=O")
    rxn_with_aam = indigo.loadQueryReaction("[C:2][S+:4]([C:3])[O-:1]>>[C:2][S:4]([C:3])=[O:1]")
    mol1 = indigo.loadMoleculeFromFile(joinPath("tests_transform_basic/ind_661/sulfoxidetest.mol"))
    mol2 = indigo.loadMoleculeFromFile(joinPath("tests_transform_basic/ind_661/sulfoxidetest.mol"))
    print(mol1.smiles())
    print("With AAM:")
    indigo.transform(rxn_with_aam, mol1)
    print(mol1.smiles())
    print("Without AAM:")
    try:
        indigo.transform(rxn_without_aam, mol2)
    except IndigoException as e:
        print('Fail: {0}'.format(getIndigoExceptionText(e)))
    print(mol2.smiles())


def testLayoutFlag(reaction):
    indigo.setOption("transform-layout", "false")
    molecule1 = indigo.loadMoleculeFromFile(joinPath("tests_transform_basic/3/mol.mol"))
    molecule2 = indigo.loadMoleculeFromFile(joinPath("tests_transform_basic/3/mol.mol"))
    indigo.transform(reaction, molecule1)
    indigo.setOption("transform-layout", "true")
    indigo.transform(reaction, molecule2)

    print("layout flag - true:")
    printCoords(molecule1)

    print("layout flag - false:")
    printCoords(molecule2)


def provideSeveralTransforms(mol, rxn_smi_path, ):
    it = indigo.iterateSmilesFile(rxn_smi_path)
    id = 0
    while it.hasNext():
        try:
            rxn = indigo.loadReactionSmarts(it.next().rawData())
            indigo.transform(rxn, mol)
            id = id + 1
        except IndigoException as e:
            print('Fail: {0}'.format(getIndigoExceptionText(e)))


def testSeveralTransforms():
    rxn_smi_path = joinPath("tests_transform_basic/4/rxns.smi")
    mol_it = indigo.iterateSmilesFile(joinPath("tests_transform_basic/4/sample_100.smi"))
    mol_id = 0
    while mol_it.hasNext():
        mol = mol_it.next()
        provideSeveralTransforms(mol, rxn_smi_path)
        mol_id = mol_id + 1

    inc_mol = indigo.loadMoleculeFromFile(joinPath("tests_transform_basic/4/inc.mol"))
    provideSeveralTransforms(inc_mol, rxn_smi_path)


test1()
test2()

print("Test3 with simple/no-layout reaction:")
reaction = indigo.loadReactionSmarts(
    "[#6:4][C@@H:2]([N,P,As,Sb,O,S,Se,Te+:3])[N,P,As,Sb,O,S,Se,Te;-:1]>>[#6:4][C@@H:2]([N,P,As,Sb,O,S,Se,Te:3])[#7,#15,#33,#51,#8,#16,#34,#52:1] |@:5|")
testLayoutFlag(reaction)

print("Test3 with layout reaction:")
reaction = indigo.loadReactionSmarts(
    "[#6:4][C@@H:2]([N,P,As,Sb,O,S,Se,Te+:3])[N,P,As,Sb,O,S,Se,Te;-:1]>>[#6:4]=[C:2][N,P,As,Sb,O,S,Se,Te:3]")
testLayoutFlag(reaction)

print("*** Transform SDF records ***")
reaction = indigo.loadQueryReaction("[S:1]>>[P:1]I")
for m in indigo.iterateSmilesFile(joinPath("../../../../../data/molecules/basic/pubchem_slice_50.smi")):
    sm1 = m.smiles()
    indigo.transform(reaction, m)
    sm2 = m.smiles()
    if sm1 != sm2:
        print(sm1 + " ->")
        print(sm2)

print("*** INDSP-197: [indigo-bugs] transform's invalid index exception ***")
indigo.setOption("ignore-stereochemistry-errors", "true")
molecule = indigo.loadMoleculeFromFile(joinPath("tests_transform_basic/indsp-197.mol"))
print(molecule.smiles())
r_remove_methyl = indigo.loadReactionSmarts("[N+:1]~[CH3]>>[N:1]")  # remove methyl
indigo.transform(r_remove_methyl, molecule)
print(molecule.smiles())

fix_n4 = indigo.loadReactionSmarts("[N;X4;!+:1]>>[N+:1]")  # add charge
indigo.transform(fix_n4, molecule)
print(molecule.smiles())

indigo.transform(r_remove_methyl, molecule)
print(molecule.smiles())

print("*** INDSP-196: [indigo-general] lengthy transform ***")
indigo = Indigo()
indigo.setOption("timeout", "4000")
molecule = indigo.loadMoleculeFromFile(joinPath("tests_transform_basic/indsp-196.mol"))
print(molecule.smiles())
reaction = indigo.loadQueryReaction(
    "[N-,P-,As-,Sb-,O-,S-,Se-,Te-:1][C:2]=[N+,P+,As+,Sb+,O+,S+,Se+,Te+:3]>>[N,P,As,Sb,O,S,Se,Te:1]=[C:2][N,P,As,Sb,O,S,Se,Te:3]")
indigo.transform(reaction, molecule)
print(molecule.smiles())

print("Several trarsforms test")
# testSeveralTransforms()
print("Ok")

print("*** INDSP-220: [indigo-bugs] transform ***")
indigo = Indigo()

transforms = [
    "[H+:10].[N-:4]1[C:5]([H,*:7])=[N:1][C:2]([H,*:8])=[C:3]1[H,*:9]>>[H:10][N:4]1[C:5]([H,*:7])=[N:1][C:2]([H,*:8])=[C:3]1[H,*:9]",
    "[H+:10].[n-:4]1[c:5]([H,*:7])[n:1][c:2]([H,*:8])[c:3]1[H,*:9]>>[H:10][N:4]1[C:5]([H,*:7])=[N:1][C:2]([H,*:8])=[C:3]1[H,*:9]"
]

methods = [
    ("loadReactionSmarts", indigo.loadReactionSmarts),
    ("loadQueryReaction", indigo.loadQueryReaction),
]

for t in transforms:
    for name, method in methods:
        print("Method: " + name)
        molecule = indigo.loadMoleculeFromFile(joinPath("tests_transform_basic/indsp-220.mol"))
        print("  Original:    " + molecule.smiles())
        rxn = method(t)
        indigo.transform(rxn, molecule)
        print("  Transformed: " + molecule.smiles())
        rxn.aromatize()
        indigo.transform(rxn, molecule)
        print("  Transformed: " + molecule.smiles())

print("*** IND-194: Stereocenter transformation***")
testInd194()

print("*** IND-661: Reaction with and without AAM***")
testInd661()
