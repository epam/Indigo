# Implementation of the method from "Prediction of Physicochemical Parameters
# by Atomic Contributions" paper
# by Scott A. Wildman and Gordon M. Crippen


import csv
from collections import Counter, defaultdict
from typing import TYPE_CHECKING, Dict, Optional

if TYPE_CHECKING:
    from indigo import IndigoObject

atom_types_table = {
    "C1": [["[CH4]", "[CH3]C", "[CH2](C)C"], 0.1441, 2.503],
    "C2": [["[CH](C)(C)C", "[C](C)(C)(C)C"], 0.0, 2.433],
    "C3": [
        ["[CH3][N,O,P,S,F,Cl,Br,I]", "[CH2X4]([N,O,P,S,F,Cl,Br,I])[A;!#1]"],
        -0.2035,
        2.753,
    ],
    "C4": [
        [
            "[CH1X4]([N,O,P,S,F,Cl,Br,I])([A;!#1])[A;!#1]",
            "[CH0X4]([N,O,P,S,F,Cl,Br,I])([A;!#1])([A;!#1])[A;!#1]",
        ],
        -0.2051,
        2.731,
    ],
    "C5": [["[C]=[!C;A;!#1]"], -0.2783, 5.007],
    "C6": [
        [
            "[CH2]=C",
            "[CH1](=C)[A;!#1]",
            "[CH0](=C)([A;!#1])[A;!#1]",
            "[C](=C)=C",
        ],
        0.1551,
        3.513,
    ],
    "C7": [["[CX2]#[A;!#1]"], 0.0017, 3.888],
    "C8": [["[CH3]c"], 0.08452, 2.464],
    "C9": [["[CH3]a"], -0.1444, 2.412],
    "C10": [["[CH2X4]a"], -0.0516, 2.488],
    "C11": [["[CHX4]a"], 0.1193, 2.582],
    "C12": [["[CH0X4]a"], -0.0967, 2.576],
    "C13": [["[cH0]-[A;!C;!N;!O;!S;!F;!Cl;!Br;!I;!#1]"], -0.5443, 4.041],
    "C14": [["[c][#9]"], 0.0, 3.257],
    "C15": [["[c][#17]"], 0.245, 3.564],
    "C16": [["[c][#35]"], 0.198, 3.18],
    "C17": [["[c][#53]"], 0.0, 3.104],
    "C18": [["[cH]"], 0.1581, 3.35],
    "C19": [["[c](:a)(:a):a"], 0.2955, 4.346],
    "C20": [["[c](:a)(:a)-a"], 0.2713, 3.904],
    "C21": [["[c](:a)(:a)-C"], 0.136, 3.509],
    "C22": [["[c](:a)(:a)-N"], 0.4619, 4.067],
    "C23": [["[c](:a)(:a)-O"], 0.5437, 3.853],
    "C24": [["[c](:a)(:a)-S"], 0.1893, 2.673],
    "C25": [["[c](:a)(:a)=[C,N,O]"], -0.8186, 3.135],
    "C26": [
        ["[C](=C)(a)[A;!#1]", "[C](=C)(c)a", "[CH1](=C)a", "[C]=c"],
        0.264,
        4.305,
    ],
    "C27": [["[CX4][A;!C;!N;!O;!P;!S;!F;!Cl;!Br;!I;!#1]"], 0.2148, 2.693],
    "C": [["[#6]"], 0.08129, 3.243],
    "H1": [["[#1][#6,#1]"], 0.123, 1.057],
    "H2": [
        ["[#1]O[CX4,c]", "[#1]O[!#6;!#7;!#8;!#16]", "[#1][!#6;!#7;!#8]"],
        -0.2677,
        1.395,
    ],
    "H3": [["[#1][#7]", "[#1]O[#7]"], 0.2142, 0.9627],
    "H4": [["[#1]OC=[#6,#7,O,S]", "[#1]O[O,S]"], 0.298, 1.805],
    "H": [["[#1]"], 0.1125, 1.112],
    "N1": [["[NH2+0][A;!#1]"], -1.019, 2.262],
    "N2": [["[NH+0]([A;!#1])[A;!#1]"], -0.7096, 2.173],
    "N3": [["[NH2+0]a"], -1.027, 2.827],
    "N4": [["[NH1+0]([!#1;A,a])a"], -0.5188, 3.0],
    "N5": [["[NH+0]=[!#1;A,a]"], 0.08387, 1.757],
    "N6": [["[N+0](=[!#1;A,a])[!#1;A,a]"], 0.1836, 2.428],
    "N7": [["[N+0]([A;!#1])([A;!#1])[A;!#1]"], -0.3187, 1.839],
    "N8": [["[N+0](a)([!#1;A,a])[A;!#1]", "[N+0](a)(a)a"], -0.4458, 2.819],
    "N9": [["[N+0]#[A;!#1]"], 0.01508, 1.725],
    "N10": [["[NH3,NH2,NH;+,+2,+3]"], -1.95, "nan"],
    "N11": [["[n+0]"], -0.3239, 2.202],
    "N12": [["[n;+,+2,+3]"], -1.119, "nan"],
    "N13": [
        [
            "[NH0;+,+2,+3]([A;!#1])([A;!#1])([A;!#1])[A;!#1]",
            "[NH0;+,+2,+3](=[A;!#1])([A;!#1])[!#1;A,a]",
            "[NH0;+,+2,+3](=[#6])=[#7]",
        ],
        -0.3396,
        0.2604,
    ],
    "N14": [
        ["[N;+,+2,+3]#[A;!#1]", "[N;-,-2,-3]", "[N;+,+2,+3](=[N;-,-2,-3])=N"],
        0.2887,
        3.359,
    ],
    "N": [["[#7]"], -0.4806, 2.134],
    "O1": [["[o]"], 0.1552, 1.08],
    "O2": [["[OH,OH2]"], -0.2893, 0.8238],
    "O3": [["[O]([A;!#1])[A;!#1]"], -0.0684, 1.085],
    "O4": [["[O](a)[!#1;A,a]"], -0.4195, 1.182],
    "O5": [["[O]=[#7,#8]", "[OX1;-,-2,-3][#7]"], 0.0335, 3.367],
    "O6": [["[OX1;-,-2,-2][#16]", "[O;-0]=[#16;-0]"], -0.3339, 0.7774],
    "O12": [["[O-]C(=O)"], -1.326, "nan"],
    "O7": [["[OX1;-,-2,-3][!#1;!N;!S]"], -1.189, 0.0],
    "O8": [["[O]=c"], 0.1788, 3.135],
    "O9": [
        [
            "[O]=[CH]C",
            "[O]=C(C)([A;!#1])",
            "[O]=[CH][N,O]",
            "[O]=[CH2]",
            "[O]=[CX2]=O",
        ],
        -0.1526,
        0.0,
    ],
    "O10": [
        ["[O]=[CH]c", "[O]=C([C,c])[a;!#1]", "[O]=C(c)[A;!#1]"],
        0.1129,
        0.2215,
    ],
    "O11": [["[O]=C([!#1;!#6])[!#1;!#6]"], 0.4833, 0.389],
    "O": [["[#8]"], -0.1188, 0.6865],
    "F": [["[#9-0]"], 0.4202, 1.108],
    "Cl": [["[#17-0]"], 0.6895, 5.853],
    "Br": [["[#35-0]"], 0.8456, 8.927],
    "I": [["[#53-0]"], 0.8857, 14.02],
    "F2": [["[#9-*]"], -2.996, "nan"],
    "Cl2": [["[#17-*]"], -2.996, "nan"],
    "Br2": [["[#35-*]"], -2.996, "nan"],
    "I2": [["[#53-*]", "[#53+*]"], -2.996, "nan"],
    "P": [["[#15]"], 0.8612, 6.92],
    "S2": [
        ["[S;-,-2,-3,-4,+1,+2,+3,+5,+6]", "[S-0]=[N,O,P,S]"],
        -0.0024,
        7.365,
    ],
    "S1": [["[S;A]"], 0.6482, 7.591],
    "S3": [["[s;a]"], 0.6237, 6.691],
    "Me1": [
        [
            "[#3,#11,#19,#37,#55]",
            "[#4,#12,#20,#38,#56]",
            "[#5,#13,#31,#49,#81]",
            "[#14,#32,#50,#82]",
            "[#33,#51,#83]",
            "[#34,#52,#84]",
        ],
        -0.3808,
        5.754,
    ],
    "Me2": [
        [
            "[#21,#22,#23,#24,#25,#26,#27,#28,#29,#30]",
            "[#39,#40,#41,#42,#43,#44,#45,#46,#47,#48]",
            "[#72,#73,#74,#75,#76,#77,#78,#79,#80]",
        ],
        -0.0025,
        "nan",
    ],
    "Hal": [
        ["[#9,#17,#35,#53;-]", "[#53;+,+2,+3]", "[+;#3,#11,#19,#37,#55]"],
        -2.996,
        "nan",
    ],
}


class TypeTable:
    """Creates dictionarys from atom type and contributions table"""

    def __init__(self, table: dict) -> None:
        self.smarts = {k: v[0] for k, v in table.items()}
        self.logp_contributions = {k: v[1] for k, v in table.items()}
        self.mr_contributions = {k: v[2] for k, v in table.items()}


TABLE = TypeTable(atom_types_table)


def calculate_mr(types_counter: Dict[str, int], table: TypeTable) -> float:
    """MR counter

    Args:
        types_counter: counted matches of atom types
        table:
    Returns:
        float: MR value
    """
    values = []
    for t, n in types_counter.items():
        values.append(table.mr_contributions[t] * n)
    return round(sum(values), 2)


def calculate_logp(types_counter: Dict[str, int], table: TypeTable) -> float:
    """LogP counter

    Args:
        types_counter: counted matches of atom types
        table:
    Returns:
        float: logP value
    """
    values = []
    for t, n in types_counter.items():
        values.append(table.logp_contributions[t] * n)
    return round(sum(values), 2)


def get_matches(
    m: "IndigoObject", table: TypeTable
) -> Optional[Dict[str, int]]:
    """Uses substructure matcher object to find matches

    Args:
        m: molecule with explicit hydrogens
        table:
    Returns:
        dict: counted matches of atom types
    """
    types_counter: Dict[str, int] = Counter()
    matcher = m.dispatcher.substructureMatcher(m)
    atoms = set()
    for atom_type, smarts in table.smarts.items():
        for i in smarts:
            query = m.dispatcher.loadSmarts(i)
            for match in matcher.iterateMatches(query):
                index = match.mapAtom(query.getAtom(0)).index()
                if index not in atoms:
                    atoms.add(index)
                    types_counter[atom_type] += 1

    if not m.countAtoms() == sum(types_counter.values()):
        return None

    return types_counter


def get_logp(mol: "IndigoObject", table: TypeTable = TABLE) -> float:
    """Returns logP value for a given molecule

    Args:
        mol (IndigoObject): molecule
        table: list of table rows
    Returns:
        float: calculated logP
    """
    m = mol.clone()
    m.unfoldHydrogens()
    types = get_matches(m, table)
    if not types:
        return 0.0
    return calculate_logp(types, table)


def get_mr(mol: "IndigoObject", table: TypeTable = TABLE) -> float:
    """Returns molar refractivity value for a given molecule

    Args:
        mol (IndigoObject): molecule
        table: list of table rows
    Returns:
        float: calculated MR
    """
    m = mol.clone()
    m.unfoldHydrogens()
    types = get_matches(m, table)
    if not types:
        return 0.0
    return calculate_mr(types, table)


def load_table_from_csv(file: str) -> TypeTable:
    """Transforms csv file into input for logP and MR calculation

    Args:
        file: csv file name
    Returns:
        instance of TypeTable object
    """
    custom_table = defaultdict(list)
    with open(file, "r", encoding="utf-8") as f:
        fields = ["type", "SMARTS", "logP", "MR"]
        reader = csv.DictReader(f, fields, delimiter=";")
        for row in reader:
            custom_table[row["type"]].append(row["SMARTS"])
            custom_table[row["type"]].append(row["logP"])
            custom_table[row["type"]].append(row["MR"])

    table_obj = TypeTable(custom_table)
    return table_obj
