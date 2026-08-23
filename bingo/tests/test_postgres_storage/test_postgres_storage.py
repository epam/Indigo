from concurrent.futures import ThreadPoolExecutor

import psycopg2
import pytest

from ..constants import DB_POSTGRES
from ..dbc.base import get_config


SCHEMA = "bingo_pg_storage_regression"
TABLE = f"{SCHEMA}.structures"
BINGO_INDEX = f"{SCHEMA}.structures_molecule_bingo"
PRODUCTION_UPDATE_VALUE = "O=C1c2ccccc2N=NN1COc1cc(Cl)c(F)cc1"


def _connect(config):
    return psycopg2.connect(
        host=config["host"],
        port=config["port"],
        dbname=config["database"],
        user=config["user"],
        password=config["password"],
    )


def _reset_schema(connection, rows=4000):
    with connection.cursor() as cursor:
        cursor.execute(f"DROP SCHEMA IF EXISTS {SCHEMA} CASCADE")
        cursor.execute(f"CREATE SCHEMA {SCHEMA}")
        cursor.execute(
            f"""
            CREATE TABLE {TABLE} (
                id bigint GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
                smiles_isomeric text NOT NULL,
                smiles_indigo text NOT NULL
            )
            """
        )
        if rows:
            cursor.execute(
                f"""
                INSERT INTO {TABLE} (smiles_isomeric, smiles_indigo)
                SELECT repeat('C', (g %% 48) + 1), repeat('C', (g %% 48) + 1)
                FROM generate_series(1, %s) AS g
                """,
                (rows,),
            )
        cursor.execute(
            f"CREATE INDEX structures_smiles_indigo_idx ON {TABLE} (smiles_indigo)"
        )
        cursor.execute(
            f"""
            CREATE INDEX structures_molecule_bingo
            ON {TABLE}
            USING bingo_idx (smiles_isomeric bingo.molecule)
            """
        )


def _assert_bingo_matches_heap(connection):
    with connection.cursor() as cursor:
        cursor.execute(
            f"SELECT count(*) FROM {TABLE} WHERE length(smiles_isomeric) >= 2"
        )
        expected = cursor.fetchone()[0]
        cursor.execute(
            f"""
            SELECT count(*)
            FROM {TABLE}
            WHERE smiles_isomeric @ ('CC', '')::bingo.sub
            """
        )
        actual = cursor.fetchone()[0]
    assert actual == expected


@pytest.fixture
def postgres_storage(request):
    if request.config.getoption("--db") != DB_POSTGRES:
        pytest.skip("PostgreSQL-specific Bingo storage regression")

    config = get_config()[DB_POSTGRES]
    connection = _connect(config)
    connection.autocommit = True

    with connection.cursor() as cursor:
        cursor.execute(
            "SELECT cvalue FROM bingo.bingo_config WHERE cname = 'NTHREADS'"
        )
        original_nthreads = cursor.fetchone()[0]
        cursor.execute(
            "UPDATE bingo.bingo_config SET cvalue = '1' WHERE cname = 'NTHREADS'"
        )

    try:
        yield connection, config
    finally:
        with connection.cursor() as cursor:
            cursor.execute(f"DROP SCHEMA IF EXISTS {SCHEMA} CASCADE")
            cursor.execute(
                "UPDATE bingo.bingo_config SET cvalue = %s WHERE cname = 'NTHREADS'",
                (original_nthreads,),
            )
        connection.close()


def test_non_hot_update_vacuum_and_reindex_preserve_bingo_results(postgres_storage):
    connection, _ = postgres_storage
    _reset_schema(connection)
    _assert_bingo_matches_heap(connection)

    # smiles_indigo has its own B-tree index, so changing it is non-HOT while
    # the Bingo key (smiles_isomeric) remains unchanged. PostgreSQL still needs
    # Bingo to represent the new heap TID.
    with connection.cursor() as cursor:
        cursor.execute(
            f"UPDATE {TABLE} SET smiles_indigo = %s",
            (PRODUCTION_UPDATE_VALUE,),
        )
        cursor.execute(f"VACUUM (INDEX_CLEANUP ON) {TABLE}")

    _assert_bingo_matches_heap(connection)

    with connection.cursor() as cursor:
        cursor.execute(f"REINDEX INDEX {BINGO_INDEX}")

    _assert_bingo_matches_heap(connection)

    with connection.cursor() as cursor:
        cursor.execute(
            f"""
            INSERT INTO {TABLE} (smiles_isomeric, smiles_indigo)
            VALUES ('CCCCCCCCCCCCCCCCCCCC', 'CCCCCCCCCCCCCCCCCCCC')
            """
        )

    _assert_bingo_matches_heap(connection)


def test_concurrent_inserts_and_non_hot_updates_keep_bingo_index_consistent(
    postgres_storage,
):
    connection, config = postgres_storage
    _reset_schema(connection, rows=0)

    workers = 8
    rows_per_worker = 300

    def insert_rows(worker):
        worker_connection = _connect(config)
        worker_connection.autocommit = True
        try:
            with worker_connection.cursor() as cursor:
                cursor.execute(
                    f"""
                    INSERT INTO {TABLE} (smiles_isomeric, smiles_indigo)
                    SELECT repeat('C', ((g + {worker}) %% 48) + 1),
                           repeat('C', ((g + {worker}) %% 48) + 1)
                    FROM generate_series(1, {rows_per_worker}) AS g
                    """
                )
        finally:
            worker_connection.close()

    with ThreadPoolExecutor(max_workers=workers) as executor:
        list(executor.map(insert_rows, range(workers)))

    _assert_bingo_matches_heap(connection)

    def update_rows(worker):
        worker_connection = _connect(config)
        worker_connection.autocommit = True
        try:
            with worker_connection.cursor() as cursor:
                cursor.execute(
                    f"""
                    UPDATE {TABLE}
                    SET smiles_indigo = smiles_indigo || 'C'
                    WHERE (id %% {workers}) = {worker}
                    """
                )
        finally:
            worker_connection.close()

    with ThreadPoolExecutor(max_workers=workers) as executor:
        list(executor.map(update_rows, range(workers)))

    with connection.cursor() as cursor:
        cursor.execute(f"VACUUM (INDEX_CLEANUP ON) {TABLE}")

    _assert_bingo_matches_heap(connection)
