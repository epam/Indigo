import argparse
import json
import sys
from pathlib import Path
from indigo import Indigo
from xml.dom.minidom import parseString

sys.path.insert(0, str(Path(__file__).parent.parent))

from tests.helpers import (
    assert_calculate_query,
    assert_match_query,
    query_cases,
    get_query_entities,
    get_bingo_meta,
)
from tests.dbc.PostgresSQL import Postgres
from tests.dbc.BingoNoSQL import BingoNoSQL
from tests.dbc.BingoElastic import BingoElastic
from tests.constants import (
    DATA_TYPES,
    DB_POSTGRES,
    DB_BINGO,
    DB_BINGO_ELASTIC,
    EntitiesType,
    TEST_CASES_DATA,
)


def update_json(test_case_file, original_data, updated_data):
    updated = False
    for original, new in zip(original_data, updated_data):
        if original["expected"] != new["expected"]:
            original["expected"] = new["expected"]
            updated = True

    if updated:
        with open(test_case_file, "w") as f:
            json.dump(original_data, f, indent=4)
    return updated


def get_indigo():
    return Indigo()


def prepare_db(function, db_option, indigo_instance):
    data_type = DATA_TYPES[function].value
    meta = get_bingo_meta(function, data_type)

    if db_option == DB_POSTGRES:
        database = Postgres()
        created_tables = []
        try:
            created_tables = database.create_data_tables(meta["tables"])
            database.import_data(import_meta=meta["import"])
            database.create_indices(meta["indices"])
        except Exception as e:
            raise e
    elif db_option == DB_BINGO:
        database = BingoNoSQL(indigo_instance)
        database.connect()
        database.import_data(meta["import_no_sql"], data_type)
        created_tables = []
    elif db_option == DB_BINGO_ELASTIC:
        from bingo_elastic.elastic import IndexName

        index_name = (
            IndexName.BINGO_REACTION
            if data_type == EntitiesType.REACTIONS.value
            else IndexName.BINGO_MOLECULE
        )
        database = BingoElastic(indigo_instance, index_name)
        database.import_data(meta["import_no_sql"], data_type)
        created_tables = []
    else:
        raise ValueError("Unsupported DB type")

    return database, created_tables


def cleanup_db(database, db_option, created_tables):
    if db_option == DB_POSTGRES:
        database.close_connect()
        for table in reversed(created_tables):
            table.drop(database.engine)
    elif db_option == DB_BINGO:
        database.delete_base()
        database.close_connect()
    elif db_option == DB_BINGO_ELASTIC:
        database.drop()
        database.close_connect()


def run_test(function, db_option, indigo_instance):
    database, created_tables = prepare_db(function, db_option, indigo_instance)
    try:
        entities_dict = get_query_entities(indigo_instance, function)
        json_file_path = Path(TEST_CASES_DATA[function])

        with open(json_file_path, "r") as f:
            original_json_data = json.load(f)

        updated_json_data = []
        unchanged = True

        test_actions = {
            "aam": lambda entity, qt: database.aam(
                entity, qt.replace("aam(", "").replace(")", "")
            ),
            "bigtable": lambda entity, _: database.substructure(
                entity, "bigtable"
            ),
            "checkmolecule": lambda entity, _: database.checkmolecule(entity),
            "checkreaction": lambda entity, _: database.checkreaction(entity),
            "cml": lambda entity, _: database.cml(entity),
            "compactmolecule": lambda entity, _: database.compactmolecule(
                entity
            ),
            "compactreaction": lambda entity, _: database.compactreaction(
                entity
            ),
            "exact": lambda entity, qt: database.exact(
                entity,
                "exact",
                None
                if qt == "exact"
                else qt[6:-1]
                if qt.startswith("exact(")
                else None,
            ),
            "fingerprint": lambda entity, qt: database.fingerprint(
                entity, qt[12:-1] if qt.startswith("fingerprint(") else None
            ),
            "gross": lambda entity, _: database.gross(entity),
            "inchi": lambda entity, qt: database.inchi(entity, inchikey=True)
            if qt.startswith("inchikey")
            else database.inchi(
                entity, qt[6:-1] if qt.startswith("inchi(") else None
            ),
            "markush": lambda entity, _: database.substructure(
                entity, "markush"
            ),
            "mass": lambda entity, qt: database.mass(entity, qt[5:-1]),
            "pseudoatoms": lambda entity, qt: (
                database.exact(entity, "pseudoatoms")
                if qt == "exact()"
                else database.exact(entity, "pseudoatoms", qt[6:-1])
                if qt.startswith("exact(")
                else database.similarity(
                    entity,
                    "pseudoatoms",
                    qt[11:-1].split(",")[0].strip(),
                    ",".join(qt[11:-1].split(",")[1:]).strip(),
                )
                if qt.startswith("similarity(")
                else database.substructure(entity, "pseudoatoms")
            ),
            "resonance": lambda entity, _: database.substructure(
                entity, "resonance", "RES"
            ),
            "rfingerprint": lambda entity, qt: database.rfingerprint(
                entity, qt[13:-1] if qt.startswith("rfingerprint(") else None
            ),
            "rsmiles": lambda entity, _: database.rsmiles(entity),
            "sgroups": lambda entity, qt: database.exact(entity, "sgroups")
            if qt.startswith("exact")
            else database.substructure(entity, "sgroups"),
            "similarity": lambda entity, _: database.similarity(
                entity, "similarity", "euclid-sub", "0.95, 1"
            ),
            "substructure": lambda entity, _: database.substructure(
                entity, "substructure"
            ),
            "tautomers": lambda entity, qt: database.exact(
                entity, "tautomers", qt[6:-1]
            )
            if qt.startswith("exact(")
            else database.substructure(
                entity,
                "tautomers",
                qt[13:-1] if qt.startswith("substructure(") else None,
            ),
        }

        assert_functions = {
            "aam": assert_calculate_query,
            "bigtable": assert_match_query,
            "checkmolecule": assert_calculate_query,
            "checkreaction": assert_calculate_query,
            "cml": assert_calculate_query,
            "compactmolecule": assert_calculate_query,
            "compactreaction": assert_calculate_query,
            "exact": assert_match_query,
            "fingerprint": assert_calculate_query,
            "gross": assert_calculate_query,
            "inchi": assert_calculate_query,
            "markush": assert_match_query,
            "mass": assert_calculate_query,
            "pseudoatoms": assert_match_query,
            "resonance": assert_match_query,
            "rfingerprint": assert_calculate_query,
            "rsmiles": assert_calculate_query,
            "sgroups": assert_match_query,
            "similarity": assert_match_query,
            "substructure": assert_match_query,
            "tautomers": assert_match_query,
        }

        for obj in original_json_data:
            query_id = obj["query_id"]
            entity = entities_dict.get(query_id)
            query_type = obj.get("query_type", "")

            if function == "inchi" and "VeryWrongOption" in query_type:
                continue

            try:
                result = test_actions[function](entity, query_type)
            except Exception as e:
                result = e

            expected = obj["expected"]
            if expected == "None\n":
                expected = None

            try:
                if isinstance(result, Exception):
                    if not isinstance(expected, str):
                        raise AssertionError(
                            "Expected must be a string if result is Exception"
                        )
                    assert_functions[function](result, expected)
                elif result is not None and expected is not None:
                    assert_functions[function](result, expected)
                elif result == expected:
                    pass
                else:
                    raise AssertionError
            except AssertionError:
                unchanged = False
                obj["expected"] = result

            updated_json_data.append(obj)

        if unchanged:
            print(f"test_{function}.py: unchanged")
        else:
            with open(json_file_path, "w") as f:
                json.dump(updated_json_data, f, indent=4)
            print(f"test_{function}.py: changed {json_file_path.name} updated")

    finally:
        cleanup_db(database, db_option, created_tables)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--db", required=True)
    parser.add_argument("--tests", help="Comma-separated list of tests to run")
    args = parser.parse_args()

    indigo_instance = get_indigo()

    all_tests = [
        "aam",
        "bigtable",
        "checkmolecule",
        "checkreaction",
        "cml",
        "compactmolecule",
        "compactreaction",
        "exact",
        "fingerprint",
        "gross",
        "inchi",
        "markush",
        "mass",
        "pseudoatoms",
        "resonance",
        "rfingerprint",
        "rsmiles",
        "sgroups",
        "similarity",
        "substructure",
        "tautomers",
    ]

    selected_tests = args.tests.split(",") if args.tests else all_tests

    for test in selected_tests:
        run_test(test, args.db, indigo_instance)
