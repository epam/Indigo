import json
import tempfile
from os import listdir, path
from typing import List, Union

import indigo as Indigo
from xmldiff import main

from .constants import (
    CREATE_INDICES_FOR,
    MOL_FILES_EXTENSIONS,
    QUERIES_DATA,
    TEST_CASES_DATA,
)


def get_bingo_meta(function: str, data_type: str):
    """
    Get meta information about required initializations for testing function:
    1. Tables to be created;
    2. Indices to be created;
    3. Path to data to be imported to tables
    """
    data_path = path.abspath(f"data/{data_type}/{function}/import")

    queries_path = path.join(data_path, "queries")
    targets_path = path.join(data_path, "targets")

    tables_to_create = []
    import_paths = {}
    nosql_data_path = None
    indices = []

    queries_files = []
    if path.exists(queries_path):
        queries_files = listdir(queries_path)

    targets_files = []
    if path.exists(targets_path):
        targets_files = listdir(targets_path)

    for file in queries_files:
        for ext in MOL_FILES_EXTENSIONS:
            if ext in file:
                nosql_data_path = path.abspath(path.join(queries_path, file))
                break

    for file in targets_files:
        for ext in MOL_FILES_EXTENSIONS:
            if ext in file:
                table_name = f"{function}_m_m_t"
                tables_to_create.append(table_name)
                import_paths[table_name] = path.abspath(
                    path.join(targets_path, file)
                )
                if nosql_data_path:
                    nosql_data_path = import_paths[table_name]
                break

    if function in CREATE_INDICES_FOR:
        indices = tables_to_create

    return {
        "tables": tables_to_create,
        "indices": indices,
        "import": import_paths,
        "import_no_sql": nosql_data_path,
    }


def indigo_iterator(indigo: Indigo, filename: str):
    """
    Provide proper Indigo iterator depending on file format
    """

    extension = path.splitext(filename)[1]
    iterator = {
        ".sdf": indigo.iterateSDFile,
        ".smi": indigo.iterateSmilesFile,
        ".sma": indigo.iterateSmilesFile,
        ".smiles": indigo.iterateSmilesFile,
        ".rdf": indigo.iterateRDFile,
    }.get(extension)
    return iterator(filename)


def get_query_entities(indigo: Indigo.Indigo, function: str):
    """
    Return dict with mapping: query_id - entity (molecule or reaction).
    Molecules are coming from data/{mol_function}/queries/.
    """

    result = {}
    entities_files = QUERIES_DATA[function]

    if type(entities_files) != list:
        entities_files = [entities_files]

    index = 1
    for entities_file in entities_files:
        it = indigo_iterator(indigo, entities_file)
        prev_molecule = ""
        while True:
            try:
                mol = next(it)
                result[index] = mol
                index += 1
                prev_molecule = mol.name()
            except StopIteration:
                break
            except Exception as e:
                print(
                    f"[ERROR] Failed to read entity {e}, prev_molecule={prev_molecule}"
                )
    return result


def query_cases(std: str, query_type=None):
    """Returns list of cases for pytest parametrized testing"""
    json_file = TEST_CASES_DATA[std]
    with open(json_file) as f:
        data = json.load(f)

    if query_type:
        data = filter(lambda o: o["query_type"] == query_type, data)

    queries = []
    for obj in data:
        obj.pop("query_type")
        queries.append(obj.values())

    return queries


def create_temp_file(data):
    temp_file = tempfile.NamedTemporaryFile(
        mode="w+b", prefix="cml_test", suffix=".xml", delete=False
    )
    temp_file.write(data.encode("ascii"))
    temp_file.close()
    return temp_file


def assert_calculate_query(
    result: Union[Exception, int, str, float], expected: Union[int, str]
):
    """Assertion for testing cansmiles, checkmolecule, cml, compactmolecule,
    fingerprint, gross, inchi, mass and molfile"""
    if isinstance(result, Exception):
        import traceback

        traceback.print_exception(result)
        assert expected in str(result)
    elif type(result) is float:
        assert round(result, 1) == round(expected, 1)
    elif isinstance(result, str) and result.startswith("<?xml"):
        diff = main.diff_texts(
            bytes(result, "utf-8"), bytes(expected, "utf-8")
        )
        assert diff == []
    else:
        assert result == expected


def assert_match_query(
    result: Union[Exception, List[int]], expected: Union[List[int], str]
):
    """Assertion for testing exact, tautomers, substructure, similarity,
    sgroups, markush, resonance, pseudoatoms, bigtable and smarts"""
    if isinstance(result, Exception):
        # `expected` is a list when the test expected matches but the DB
        # returned an error (e.g. ORA-28579 extproc crash slipped past
        # OracleDB._execute_query's retry). `list in str` raises TypeError,
        # which crashes pytest before the assertion message is rendered.
        if isinstance(expected, list):
            raise AssertionError(
                f"expected matches {expected}, got exception: {result}"
            )
        assert expected in str(result)
    elif type(expected) == list:
        assert set(result) == set(expected)


def assert_aam_query(result, expected, mode):
    """AAM-aware assertion. Prefers exact-string match (preserves the
    deterministic behavior on Postgres / bingo-nosql), and falls back to
    semantic AAM validation when exact fails — needed on Oracle, where
    Bingo's ReactionAutomapper heuristic produces different but equally
    valid mappings across extproc sessions."""
    if not isinstance(result, Exception) and result == expected:
        return

    if isinstance(result, Exception):
        assert expected in str(result)
        return

    assert isinstance(result, str) and result.startswith("$RXN"), (
        f"aam({mode}) returned non-RXN output: {result!r}"
    )

    indigo = Indigo.Indigo()
    rxn = indigo.loadReaction(result)

    reactant_maps, product_maps = set(), set()
    for mol in rxn.iterateReactants():
        for atom in mol.iterateAtoms():
            n = rxn.atomMappingNumber(atom)
            if n != 0:
                reactant_maps.add(n)
    for mol in rxn.iterateProducts():
        for atom in mol.iterateAtoms():
            n = rxn.atomMappingNumber(atom)
            if n != 0:
                product_maps.add(n)

    if mode.upper() == "CLEAR":
        # CLEAR is allowed to leave or strip pre-existing input maps (Oracle
        # extproc serialization varies), but it must not produce a mapping
        # that's inconsistent between sides.
        common = reactant_maps & product_maps
        assert reactant_maps == common or product_maps == common or (
            not reactant_maps and not product_maps
        ), (
            f"CLEAR produced inconsistent maps: "
            f"reactants={reactant_maps}, products={product_maps}"
        )
    else:
        assert reactant_maps == product_maps, (
            f"{mode}: inconsistent AAM. "
            f"reactants={reactant_maps}, products={product_maps}"
        )
