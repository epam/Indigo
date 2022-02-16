# Implementation of the method from "Prediction of Physicochemical Parameters
# by Atomic Contributions" paper
# by Scott A. Wildman and Gordon M. Crippen


import csv
from collections import Counter, defaultdict
from typing import Dict, List

from smarts_table import TABLE  # type: ignore

from indigo import Indigo, IndigoObject  # type: ignore

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", True)
indigo.setOption("ignore-bad-valence", True)


def get_smarts_dict(types_smarts: List[str]) -> Dict[str, List]:
    """Creates dictionary from list of table rows

    Args:
        types_smarts: list of table rows
    Returns:
        dictionary of smarts, where keys are atom types
        and values are smarts
    """
    smarts_dict = defaultdict(list)
    for string in types_smarts:
        row = string.split()
        smarts_dict[row[0]].append(row[1])
    return smarts_dict


def get_logp_contributions_dict(types_smarts: List[str]) -> Dict[str, float]:
    """Creates dictionary from list of table rows

    Args:
        types_smarts: list of table rows
    Returns:
        dictionary of logP contributions to atom types
    """
    contrib_dict = {}
    for string in types_smarts:
        row = string.split()
        contrib_dict[row[0]] = float(row[2])
    return contrib_dict


def get_mr_contributions_dict(types_smarts: List[str]) -> Dict[str, float]:
    """Creates dictionary from list of table rows

    Args:
        types_smarts: list of table rows
    Returns:
        dictionary of MR contributions to atom types
    """
    contrib_dict = {}
    for string in types_smarts:
        row = string.split()
        contrib_dict[row[0]] = float(row[3])
    return contrib_dict


def count_mr(types_counter: Dict[str, int], table: List[str]) -> float:
    """MR counter

    Args:
        types_counter: counted matches of atom types
        table: counted matches of atom types
    Returns:
        float: MR value
    """
    values = []
    mr_contributions = get_mr_contributions_dict(table)
    for t, n in types_counter.items():
        values.append(mr_contributions[t] * n)
    return sum(values)


def count_logp(types_counter: Dict[str, int], table: List[str]) -> float:
    """LogP counter

    Args:
        types_counter: counted matches of atom types
        table: counted matches of atom types
    Returns:
        float: logP value
    """
    values = []
    logp_contributions = get_logp_contributions_dict(table)
    for t, n in types_counter.items():
        values.append(round(logp_contributions[t] * n, 2))
    return sum(values)


def get_matches(mol: IndigoObject, types_smarts: List[str]) -> Dict[str, int]:
    """Uses substructure matcher object to find matches

    Args:
        mol: molecule
        types_smarts: list of table rows
    Returns:
        dict: counted matches of atom types
    """
    types_counter: Dict[str, int] = Counter()
    matcher = indigo.substructureMatcher(mol)
    atoms = set()
    d = get_smarts_dict(types_smarts)
    for atom_type, smarts in d.items():
        for i in smarts:
            query = indigo.loadSmarts(i)
            for match in matcher.iterateMatches(query):
                index = match.mapAtom(query.getAtom(0)).index()
                if index not in atoms:
                    atoms.add(index)
                    types_counter[atom_type] += 1

    return types_counter


def get_logp(mol: IndigoObject, table: List[str] = TABLE) -> float:
    """Returns logP value for a given molecule

    Args:
        mol (IndigoObject): molecule
        table: list of table rows
    Returns:
        float: calculated logP
    """
    mol.unfoldHydrogens()
    types = get_matches(mol, table)
    return count_logp(types, table)


def get_mr(mol: IndigoObject, table: List[str] = TABLE) -> float:
    """Returns molar refractivity value for a given molecule

    Args:
        mol (IndigoObject): molecule
        table: list of table rows
    Returns:
        float: calculated MR
    """
    mol.unfoldHydrogens()
    types = get_matches(mol, table)
    return count_mr(types, table)


def load_table_from_csv(file: str) -> List[str]:
    """Transforms csv file into input for logP and MR calculation"""
    custom_table = []
    with open(file, "r", encoding="utf-8") as f:
        fields = ["type", "SMARTS", "logP", "MR"]
        reader = csv.DictReader(f, fields, delimiter=";")
        for row in reader:
            s = f'{row["type"]} {row["SMARTS"]} {row["logP"]} {row["MR"]}'
            custom_table.append(s)

    return custom_table
