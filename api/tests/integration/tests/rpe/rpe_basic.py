import sys

sys.path.append('../../common')
from env_indigo import Indigo, IndigoException, getIndigoExceptionText, joinPath

indigo = Indigo()


def reactionProdEnum(molset, reaction):
    output_reactions = indigo.reactionProductEnumerate(reaction, [molset])
    for out_rxn in output_reactions.iterateArray():
        for mol in out_rxn.iterateProducts():
            prod = mol.clone()
            prod.setName(out_rxn.name())
            yield prod


def reactionProdEnumSet(set_of_molset, reaction):
    output_reactions = indigo.reactionProductEnumerate(reaction, set_of_molset)
    for out_rxn in output_reactions.iterateArray():
        for mol in out_rxn.iterateProducts():
            prod = mol.clone()
            prod.setName(out_rxn.name())
            yield prod


def transform(molset, reaction):
    for mol in molset:
        m = mol.clone()
        mapping = indigo.transform(reaction, m)
        yield [m, mapping]


def transformByString(reaction_string, molecules_smiles, smarts_flag, transform_flag):
    molset = []
    for m, idx in zip(molecules_smiles, range(len(molecules_smiles))):
        mol = indigo.loadMolecule(m)
        mol.setName("mol_%d" % idx)
        molset.append(mol)
        # print(mol.molfile())

    if smarts_flag:
        reaction = indigo.loadReactionSmarts(reaction_string)
    else:
        reaction = indigo.loadQueryReaction(reaction_string)

    try:
        res = []

        if transform_flag:
            for [mol, mapping] in transform(molset, reaction):
                res.append((mol.canonicalSmiles(), mol.name(), mapping))
        else:
            for mol in reactionProdEnum(molset, reaction):
                res.append((mol.canonicalSmiles(), mol.name(), None))

        idx = 0
        for smiles, name, mapping in sorted(res, reverse=True):
            print("    %s: %s" % (name, smiles))
            if mapping != None:
                for atom in molset[idx].iterateAtoms():
                    if (mapping.mapAtom(atom) != None):
                        print("[" + str(idx) + "] " + str(atom.index()) + "-" + str(mapping.mapAtom(atom).index()))
            idx += 1
    except IndigoException as e:
        msg = "Error: %s" % (getIndigoExceptionText(e))
        print(msg)


def startRPE(reaction_path, molecule_paths, is_layout):
    print("***")
    reaction = indigo.loadQueryReactionFromFile(reaction_path)
    indigo.setOption("rpe-layout", is_layout)
    print("Reaction: %s" % reaction.smiles())

    print("Monomers:")
    molset = []
    for m, idx in zip(molecule_paths, range(len(molecule_paths))):
        mol = indigo.loadMoleculeFromFile(m)
        print("    %s: %s" % ("mol_%d" % idx, mol.smiles()))
        mol.setName("mol_%d" % idx)
        molset.append([mol])

    try:
        res = []

        for mol in reactionProdEnumSet(molset, reaction):
            res.append((mol, mol.canonicalSmiles(), mol.name()))

        for mol, smiles, name in sorted(res, reverse=True, key=lambda x: x[2]):
            print("    %s: %s hasXYZ: %d" % (name, smiles, mol.hasCoord()))

        print("\n")

    except IndigoException as e:
        msg = "Error: %s" % (getIndigoExceptionText(e))
        print(msg)
    finally:
        indigo.setOption("rpe-layout", True)


def reactBySmiles(reaction_smiles, molecules_smiles):
    print("rpe+smiles:")
    transformByString(reaction_smiles, molecules_smiles, False, False)


def reactBySmarts(reaction_smarts, molecules_smiles):
    print("rpe+smarts:")
    transformByString(reaction_smarts, molecules_smiles, True, False)


def transformBySmiles(reaction_smiles, molecules_smiles):
    print("transform+smiles:")
    transformByString(reaction_smiles, molecules_smiles, False, True)


def transformBySmarts(reaction_smarts, molecules_smiles):
    print("transform+smarts:")
    transformByString(reaction_smarts, molecules_smiles, True, True)


def testTransformation(reaction_string, molecules_smiles):
    print("***")
    print("Reaction: %s" % reaction_string)
    print("Monomers:")
    molset = []
    for m, idx in zip(molecules_smiles, range(len(molecules_smiles))):
        print("    %s: %s" % ("mol_%d" % idx, m))

    print("Products:")
    reactBySmiles(reaction_string, molecules_smiles)
    reactBySmarts(reaction_string, molecules_smiles)
    transformBySmiles(reaction_string, molecules_smiles)
    transformBySmarts(reaction_string, molecules_smiles)
    print("\n")


def testTransformationMolfile(reaction_string, molfile_path):
    molecule = indigo.loadMoleculeFromFile(molfile_path)
    testTransformation(reaction_string, [molecule.smiles()])
    print("\n")


testTransformation("O[*:1]>>[*:1][H]", ["C(O)C=C(O)C(O)", "OC(=C)C1=CC=CC=C1O"])
testTransformation("[H]C([*:2])([*:3])C([*:5])([*:6])[*:7]>>C([*:2])([*:3])=C([*:5])([*:6])",
                   ["C1(Cl)C(Cl)C(Cl)C(Cl)C(Cl)C1(Cl)"])
testTransformation("[H]C([H,*:2])([H,*:3])C([H,*:5])([H,*:6])[H,*:7]>>C([H,*:2])([H,*:3])=C([H,*:5])([H,*:6])",
                   ["C1(Cl)C(Cl)C(Cl)C(Cl)C(Cl)C1(Cl)"])
testTransformation("[H]C([H,*:2])([H,*:3])C([H,*:5])([H,*:6])[H,*:7]>>C([*:2])([*:3])=C([*:5])([H,*:6])",
                   ["C1(Cl)C(Cl)C(Cl)C(Cl)C(Cl)C1(Cl)"])
testTransformation("[*:1]C([*:2])=[*:3]>>[*:1]C=[*:3]", ["C=1(Cl)C=CC=CC1"])
testTransformation("[*:2]c(:[*:1]):[*:3]>>[*:1]:c:[*:3]", ["C=1(Cl)C=CC=CC1"])
testTransformation("[*:1]>>[*;+:1]", ["C", "[H]"])
testTransformation("[*:1]>>[*;+0:1]", ["[C+]", "[H]"])
testTransformation("[*:1]>>[O,N;+:1]", ["O", "N", "C"])
# Bug?: Incorrect AAM
testTransformation("[H,*:2]>>N[*:2]", ["C", ])
testTransformation("[H,*:1]>>N[H,*:1]", ["C"])
# Bug!: array: invalid index 14 (size=14)
testTransformation("[H,*:1]>>[*:1]", ["C[2H]", ])
testTransformation("[H,*:1]>>[H,*:1]", ["C[2H]"])
testTransformation("[H,*:1]>>[*:1]", ["[H]", ])
testTransformation("[H,*:1]>>[H,*:1]", ["[H]"])
testTransformation(
    "[H:1][O,S,Se,Te:2][C:3]([H,*:7])=[C:4]([H,*:6])[H,*:5]>>[H:1][C:4]([H,*:6])([H,*:5])[C:3]([H,*:7])=[O,S,Se,Te:2]",
    ["C[C@]1(O)[C@H]2C[C@H]3[C@@H](C(O)=C(C(N)=O)C(=O)[C@@]3(O)C(=O)C2C(=O)C2=C1C=CC=C2O)N(C)C"])
testTransformation("[H]c1[c:5][c:4][c:3][c:2]c1C(=O)N[H,*:1]>>[H,*:1]NC1=NOc2c1[c:2][c:3][c:4][c:5]2",
                   ["NC(=O)c1ccccc1"])
testTransformation("[*:1]:[c]([*:2]):[c:10]([*:3]):[n]([*:4]):[*:5]>>[*:1]:[n]([*:2]):[c:10]([*:3]):[c]([*:4])[*:5]",
                   ["Oc1ccn(C)c1C"])
testTransformation("[!O:1][S;X3:2](=[O:3])[!O:4]>>[!O:1][S+;X3:2]([O-:3])[!O:4]", ["CNC(=O)ON=CC(C)(C)S(C)=O"])
testTransformationMolfile(
    "[H:1][O:2][C:3]([H,*:4])=[C:5]([H,*;!O:6])[H,*;!O:7]>>[O:2]=[C:3]([H,*:4])[C:5]([H,*:6])([H:1])[H,*:7]",
    joinPath("tests_rpe_basic/1/reactant.mol"))
testTransformationMolfile("[N+:1]~[CH3]>>[N:1]", joinPath("tests_rpe_basic/2/mol.mol"))

startRPE(joinPath("tests_rpe_basic/ind_671/reaction.rxn"),
         [joinPath("tests_rpe_basic/ind_671/P1.mol"), joinPath("tests_rpe_basic/ind_671/P2.mol"),
          joinPath("tests_rpe_basic/ind_671/P3.mol")], True)
startRPE(joinPath("tests_rpe_basic/ind_671/reaction.rxn"),
         [joinPath("tests_rpe_basic/ind_671/P1.smi"), joinPath("tests_rpe_basic/ind_671/P2.smi"),
          joinPath("tests_rpe_basic/ind_671/P3.smi")], True)

startRPE(joinPath("tests_rpe_basic/3/rxn.smi"), [joinPath("tests_rpe_basic/3/mol.mol")], True)
startRPE(joinPath("tests_rpe_basic/3/rxn.smi"), [joinPath("tests_rpe_basic/3/mol.mol")], False)
