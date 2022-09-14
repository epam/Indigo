import os
import sys
from collections import defaultdict


sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("ignore-bad-valence", True)
indigo.setOption("ignore-stereochemistry-errors", True)
indigo.setOption("molfile-saving-add-implicit-h", False)


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

pka_data = defaultdict(dict)
with open(dataPath("molecules/pka/pKaInWater.csv")) as f:
    next(f)
    for line in f:
        try:
            smiles, exp_pka_value, pka_type = line.split(",")
            exp_pka_value = float(exp_pka_value)
            pka_type = pka_type.strip()
            m = indigo.loadMolecule(smiles)
            if pka_type == "acidic":
                if smiles in pka_data and "acidic" in pka_data[smiles]:
                    print("duplicate acidic " + smiles)
                    continue
                pkas = {}

                matcher = indigo.substructureMatcher(m)
                for acid_smarts, pka_value, _ in table:
                    q = indigo.loadQueryMolecule(acid_smarts)
                    match = matcher.match(q)
                    if match:
                        pkas[match.mapAtom(q.getAtom(0)).index()] = pka_value

                min_atom_idx = None
                min_pka_value = 1000.0
                for idx, pka in pkas.items():
                    if pka < min_pka_value:
                        min_pka_value = pka
                        min_atom_idx = idx
                if min_atom_idx and min_pka_value < 10.0:
                    pka_data[smiles]["acidic"] = (min_atom_idx, exp_pka_value)
            elif pka_type == "basic":
                if smiles in pka_data and "basic" in pka_data[smiles]:
                    print("duplicate basic " + smiles)
                    continue
                pkas = {}

                matcher = indigo.substructureMatcher(m)
                for _, pka_value, basic_smarts in table:
                    q = indigo.loadQueryMolecule(basic_smarts)
                    match = matcher.match(q)
                    if match:
                        pkas[match.mapAtom(q.getAtom(0)).index()] = pka_value

                max_atom_idx = None
                max_pka_value = -1000.0
                for idx, pka in pkas.items():
                    if pka > max_pka_value:
                        max_pka_value = pka
                        max_atom_idx = idx
                if max_atom_idx and max_pka_value > -10.0:
                    pka_data[smiles]["basic"] = (max_atom_idx, exp_pka_value)
            else:
                raise ValueError(line)
        except IndigoException as e:
            print(e, line)

print(pka_data)
saver = indigo.createFileSaver(
    dataPath("molecules/pka/pka_in_water.sdf"), "sdf"
)
for smiles in pka_data:
    try:
        mol = indigo.loadMolecule(smiles)
        if "acidic" in pka_data[smiles]:
            mol.setProperty("ACID PKA SITES", str(pka_data[smiles]["acidic"][0]))
            mol.setProperty("ACID PKA VALUES", str(pka_data[smiles]["acidic"][1]))
        if "basic" in pka_data[smiles]:
            mol.setProperty("BASIC PKA SITES", str(pka_data[smiles]["basic"][0]))
            mol.setProperty("BASIC PKA VALUES", str(pka_data[smiles]["basic"][1]))
        saver.append(mol)
    except IndigoException as e:
        print(e, smiles)
