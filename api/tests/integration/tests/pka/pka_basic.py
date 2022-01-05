import sys

sys.path.append("../../common")
from env_indigo import *

indigo = Indigo()
indigo.setOption("molfile-saving-skip-date", "true")
indigo.setOption("molfile-saving-mode", "3000")

renderer = IndigoRenderer(indigo)
collection = indigo.createArray()

acids = []
basics = []
pKas = []

table = [
    ("[F;!H0]", 3.18, "[F-]"),
    ("[Cl;!H0]", -6.50, "[Cl-]"),
    ("[Br;!H0]", -8.50, "[Br-]"),
    ("[I;!H0]", -9.00, "[I-]"),
    ("[c;!H0]", 43.00, "[c-]"),
    ("[$([C]#N);!H0]", 9.30, "[$([C-]#N)]"),
    ("[C;!H0]", 50.00, "[C-]"),
    ("[nH;!H0]", 16.50, "[n-]"),
    ("[$([N]=N=*);!H0]", -99.99, "[$([N-]=N=*)]"),
    ("[$([N]C=O);!H0]", 22.00, "[$([N-]C=O)]"),
    ("[$([N]S(=O)=O);!H0]", 10.10, "[$([N-]S(=O)=O)]"),
    ("[N;!H0]", 32.50, "[N-]"),
    ("[nH2+;!H0]", -3.80, "[nH]"),
    ("[nH+;!H0]", 5.23, "[n]"),
    ("[$([NH+]#*);!H0]", -12.00, "[$([N]#*)]"),
    ("[$([NH+]=C(N)N);!H0]", 14.4, "[$([N]=C(N)N)]"),
    ("[$([NH+]=C(N)a);!H0]", 11.6, "[$([N]=C(N)a)]"),
    ("[$([NH+]=CN);!H0]", 12.4, "[$([N]=CN)]"),
    ("[$([NH+]=*);!H0]", -99.99, "[$([N]=*)]"),
    ("[$([NH+]a);!H0]", 4.69, "[$([N]a)]"),
    ("[$([NH+]C=O);!H0]", 4.74, "[$([N]C=O)]"),
    ("[$([NH+]C=N);!H0]", -99.99, "[$([N]C=N)]"),
    ("[$([NH+]S(=O)=O);!H0]", -99.99, "[$([N]S(=O)=O)]"),
    ("[NH+;!H0]", 10.5, "[N]"),
    ("[NH2+;!H0]", 11.1, "[$([N](C)C)]"),
    ("[NH3+;!H0]", 10.6, "[NH2]"),
    ("[NH4+;!H0]", 9.25, "[NH3]"),
    ("[OH2;!H0]", 15.70, "[OH-]"),
    ("[$([O]c);!H0]", 10.00, "[O-]a"),
    ("[$([O]C(=O)[O-]);!H0]", 10.33, "[$([O-]C(=O)[O-])]"),
    ("[$([O]C(=O)a);!H0]", 4.20, "[$([O-]C(=O)a)]"),
    ("[$([O]C=O);!H0]", 4.80, "[$([O-]C=O)]"),
    ("[$([O]C);!H0]", 15.50, "[$([O-]C)]"),
    ("[$([O]N(=O)=O);!H0]", -1.40, "[$([O-]N(=O)=O)]"),
    ("[$([O][N+]=O);!H0]", -12.00, "[$([O-][N+]=O)]"),
    ("[$([O]NC=O);!H0]", 9.40, "[$([O-]NC=O)]"),
    ("[$([O]N=*);!H0]", 12.34, "[$([O-]N=*)]"),
    ("[$([O]N(*)*);!H0]", 5.2, "[$([O-]N(*)*)]"),
    ("[$([O]N);!H0]", 5.96, "[$([O-]N)]"),
    ("[$([O]P([O-])([O-]));!H0]", 12.50, "[$([O-]P([O-])([O-]))]"),
    ("[$([O]P([O-])=O);!H0]", 6.70, "[$([O-]P([O-])=O)]"),
    ("[$([O]P=O);!H0]", 2.00, "[$([O-]P=O)]"),
    ("[$([O]P[O-]);!H0]", 99.99, "[$([O-]P[O-])]"),
    ("[$([O]Pa);!H0]", 2.10, "[$([O-]Pa)]"),
    ("[$([O]P);!H0]", 3.08, "[$([O-]P)]"),
    ("[$([O]S(=O)(=O)[O-]);!H0]", 2.0, "[$([O-]S(=O)(=O)[O-])]"),
    ("[$([O]S(=O)(=O));!H0]", -99.99, "[$([O-]S(=O)(=O))]"),
    ("[$([O]S(=O)[O-]);!H0]", 7.20, "[$([O-]S(=O)[O-])]"),
    ("[$([O]S(=O));!H0]", 1.80, "[$([O-]S(=O))]"),
    ("[O;!H0]", 99.99, "[O-]"),
    ("[OH+;!H0]", -1.74, "[O]"),
    ("[P;!H0]", 29.00, "[P-]"),
    ("[P+;!H0]", -13.00, "[P]"),
    ("[$([S]*=O);!H0]", 3.52, "[$([S-]*=O)]"),
    ("[$([S]a);!H0]", 6.52, "[$([S-]a)]"),
    ("[SH2;!H0]", 7.00, "[SH-]"),
    ("[S;!H0]", 12.00, "[S-]"),
    ("[SH+;!H0]", -7.00, "[S]"),
]

for acid, pka, basic in table:
    acids.append(acid)
    pKas.append(pka)
    basics.append(basic)

pH = 7.0
pH_toll = 0.5

for root, dirnames, filenames in os.walk(joinPathPy("molecules/AA", __file__)):
    filenames.sort()
    for filename in filenames:
        acid_sites = []
        basic_sites = []
        acid_pKas = []
        basic_pKas = []

        mol = indigo.loadMoleculeFromFile(os.path.join(root, filename))

        matcher = indigo.substructureMatcher(mol)
        for acid in acids:
            atoms = []
            q = indigo.loadSmarts(acid)
            for match in matcher.iterateMatches(q):
                if match != None:
                    for atom in q.iterateAtoms():
                        if match.mapAtom(atom) != None:
                            matched_atom = match.mapAtom(atom)
                            acid_sites.append(matched_atom)
                            acid_pKas.append(pKas[acids.index(acid)])
                            atoms.append(matched_atom)
            for atom in atoms:
                matcher.ignoreAtom(atom)

        matcher = indigo.substructureMatcher(mol)
        for basic in basics:
            atoms = []
            q = indigo.loadSmarts(basic)
            for match in matcher.iterateMatches(q):
                if match != None:
                    for atom in q.iterateAtoms():
                        if match.mapAtom(atom) != None:
                            matched_atom = match.mapAtom(atom)
                            basic_sites.append(matched_atom)
                            basic_pKas.append(pKas[basics.index(basic)])
                            atoms.append(matched_atom)
            for atom in atoms:
                matcher.ignoreAtom(atom)

        print("%s" % filename[:-4])

        for atom in acid_sites:
            if (acid_pKas[acid_sites.index(atom)] - pH) < pH_toll:
                atom.setCharge(atom.charge() - 1)
                print("              %s" % acid_pKas[acid_sites.index(atom)])

        for atom in basic_sites:
            if (basic_pKas[basic_sites.index(atom)] - pH) > -pH_toll:
                atom.setCharge(atom.charge() + 1)
                print("              %s" % basic_pKas[basic_sites.index(atom)])

        mol.setProperty("grid-comment", filename)
        collection.arrayAdd(mol)

indigo.setOption("render-coloring", "true")
indigo.setOption("render-bond-length", "48")
indigo.setOption("render-grid-title-font-size", "18")
indigo.setOption("render-grid-margins", "20, 10")
indigo.setOption("render-grid-title-offset", "5")
indigo.setOption("render-grid-title-property", "grid-comment")

if not os.access(joinPathPy("out", __file__), os.F_OK):
    os.makedirs(joinPathPy("out", __file__))

renderer.renderGridToFile(
    collection, None, 4, joinPathPy("out/ionize1.png", __file__)
)

collection.clear()

for root, dirnames, filenames in os.walk(joinPathPy("molecules/AA", __file__)):
    filenames.sort()
    for filename in filenames:
        mol = indigo.loadMoleculeFromFile(os.path.join(root, filename))
        #        print(mol.molfile())
        mol.ionize(pH, pH_toll)
        #        print(mol.molfile())
        mol.setProperty("grid-comment", filename)
        collection.arrayAdd(mol)

indigo.setOption("render-coloring", "true")
indigo.setOption("render-bond-length", "48")
indigo.setOption("render-grid-title-font-size", "18")
indigo.setOption("render-grid-margins", "20, 10")
indigo.setOption("render-grid-title-offset", "5")
indigo.setOption("render-grid-title-property", "grid-comment")

renderer.renderGridToFile(
    collection, None, 4, joinPathPy("out/ionize2.png", __file__)
)

collection.clear()

level = indigo.buildPkaModel(
    10, 0.5, joinPathPy("molecules/PkaModel.sdf", __file__)
)
# level = indigo.buildPkaModel(10, 0.5, joinPathPy('test.sdf', __file__))

indigo.setOption("pKa-model", "advanced")
indigo.setOption("pKa-model-level", 5)
indigo.setOption("pKa-model-min-level", 2)

for root, dirnames, filenames in os.walk(joinPathPy("molecules/AA", __file__)):
    filenames.sort()
    for filename in filenames:
        mol = indigo.loadMoleculeFromFile(os.path.join(root, filename))
        #        print(mol.molfile())
        print("%s" % filename[:-4])

        #        mol.aromatize()
        for atom in mol.iterateAtoms():
            a_pka = mol.getAcidPkaValue(atom, 5, 2)
            b_pka = mol.getBasicPkaValue(atom, 5, 2)
            if a_pka < 100.0:
                print(
                    "Acid site on atom %d " % atom.index()
                    + " has pKa value = %4.2f" % a_pka
                )
            if b_pka > -100.0:
                print(
                    "Basic site on atom %d " % atom.index()
                    + " has pKa value = %4.2f" % b_pka
                )

            #        mol.dearomatize()

        mol.ionize(pH, pH_toll)
        #        print(mol.molfile())
        mol.setProperty("grid-comment", filename)
        collection.arrayAdd(mol)

indigo.setOption("render-coloring", "true")
indigo.setOption("render-bond-length", "48")
indigo.setOption("render-grid-title-font-size", "18")
indigo.setOption("render-grid-margins", "20, 10")
indigo.setOption("render-grid-title-offset", "5")
indigo.setOption("render-grid-title-property", "grid-comment")

renderer.renderGridToFile(
    collection, None, 4, joinPathPy("out/ionize3.png", __file__)
)

level = indigo.buildPkaModel(
    10, 0.5, joinPathPy("molecules/adv_pka_model.sdf", __file__)
)

for item in indigo.iterateSmilesFile(
    joinPathPy("molecules/test_set.smi", __file__)
):
    #    print("%s" % item.rawData())
    item.dearomatize()
    mol = indigo.loadMolecule(item.canonicalSmiles())
    pka_found = False
    pkas_list = []
    for atom in mol.iterateAtoms():
        a_pka = mol.getAcidPkaValue(atom, 10, 2)
        b_pka = mol.getBasicPkaValue(atom, 10, 2)
        if a_pka < 100.0:
            pkas_list.append(a_pka)
            pka_found = True
        if b_pka > -100.0:
            pkas_list.append(b_pka)
            pka_found = True
    if pka_found == False:
        print("%s" % item.rawData())
    else:
        pka_str = ""
        for pka in pkas_list:
            pka_str = pka_str + ",%4.2f" % pka
        print("%s" % item.rawData() + "%s" % pka_str)

for item in indigo.iterateSDFile(
    joinPathPy("molecules/adv_pka_model.sdf", __file__)
):
    print("%s" % item.smiles())
    mol = indigo.loadMolecule(item.rawData())
    mol.foldHydrogens()
    for atom in mol.iterateAtoms():
        a_pka = mol.getAcidPkaValue(atom, 5, 2)
        b_pka = mol.getBasicPkaValue(atom, 5, 2)
        if a_pka < 100.0:
            print(
                "======= Acid site on atom %d " % atom.index()
                + " has pKa value = %4.2f" % a_pka
            )
        if b_pka > -100.0:
            print(
                "======= Basic site on atom %d " % atom.index()
                + " has pKa value = %4.2f" % b_pka
            )
if isIronPython():
    renderer.Dispose()
