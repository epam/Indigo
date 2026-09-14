from concurrent.futures import ThreadPoolExecutor

import psycopg2
import pytest

from ..constants import DB_POSTGRES
from ..dbc.base import get_config

SCHEMA = "bingo_pg_storage_regression"
TABLE = f"{SCHEMA}.structures"
BINGO_INDEX = f"{SCHEMA}.structures_molecule_bingo"
SHADOW_TABLE = f"{BINGO_INDEX}_shadow"
SHADOW_HASH_TABLE = f"{BINGO_INDEX}_shadow_hash"
PRODUCTION_UPDATE_VALUE = "O=C1c2ccccc2N=NN1COc1cc(Cl)c(F)cc1"
INVALID_SMILES = "C1CC"
VALID_EXACT_SMILES = "CCO"


def _connect(config):
    return psycopg2.connect(
        host=config["host"],
        port=config["port"],
        dbname=config["database"],
        user=config["user"],
        password=config["password"],
    )


def _create_bingo_index(
    connection,
    *,
    nthreads=None,
    reject_invalid_structures=None,
):
    options = []
    if nthreads is not None:
        options.append(f"NTHREADS = {nthreads}")
    if reject_invalid_structures is not None:
        options.append(
            f"REJECT_INVALID_STRUCTURES = {reject_invalid_structures}"
        )

    with_clause = f"WITH ({', '.join(options)})" if options else ""
    with connection.cursor() as cursor:
        cursor.execute(f"""
            CREATE INDEX structures_molecule_bingo
            ON {TABLE}
            USING bingo_idx (smiles_isomeric bingo.molecule)
            {with_clause}
            """)


def _reset_schema(connection, rows=4000, create_bingo_index=True):
    with connection.cursor() as cursor:
        cursor.execute(f"DROP SCHEMA IF EXISTS {SCHEMA} CASCADE")
        cursor.execute(f"CREATE SCHEMA {SCHEMA}")
        cursor.execute(f"""
            CREATE TABLE {TABLE} (
                id bigint GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
                smiles_isomeric text NOT NULL,
                smiles_indigo text NOT NULL
            )
            """)
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

    if create_bingo_index:
        _create_bingo_index(connection)


def _assert_bingo_matches_heap(connection):
    with connection.cursor() as cursor:
        cursor.execute(
            f"SELECT count(*) FROM {TABLE} WHERE length(smiles_isomeric) >= 2"
        )
        expected = cursor.fetchone()[0]
        cursor.execute(f"""
            SELECT count(*)
            FROM {TABLE}
            WHERE smiles_isomeric @ ('CC', '')::bingo.sub
            """)
        actual = cursor.fetchone()[0]
    assert actual == expected


def _assert_bingo_matches_indexable_heap(connection):
    with connection.cursor() as cursor:
        cursor.execute(f"""
            SELECT count(*)
            FROM {TABLE}
            WHERE length(smiles_isomeric) >= 2
              AND bingo.checkmolecule(smiles_isomeric) IS NULL
            """)
        expected = cursor.fetchone()[0]
        cursor.execute(f"""
            SELECT count(*)
            FROM {TABLE}
            WHERE smiles_isomeric @ ('CC', '')::bingo.sub
            """)
        actual = cursor.fetchone()[0]
    assert actual == expected


def _assert_exact_count(connection, smiles, expected):
    with connection.cursor() as cursor:
        cursor.execute(
            f"""
            SELECT count(*)
            FROM {TABLE}
            WHERE smiles_isomeric @ (%s, '')::bingo.exact
            """,
            (smiles,),
        )
        actual = cursor.fetchone()[0]
    assert actual == expected


def _shadow_counts(connection):
    with connection.cursor() as cursor:
        cursor.execute(f"SELECT count(*) FROM {SHADOW_TABLE}")
        shadow_count = cursor.fetchone()[0]
        cursor.execute(f"SELECT count(*) FROM {SHADOW_HASH_TABLE}")
        shadow_hash_count = cursor.fetchone()[0]
    return shadow_count, shadow_hash_count


def _assert_no_zero_pages(connection):
    with connection.cursor() as cursor:
        cursor.execute("CHECKPOINT")
        cursor.execute(
            """
            WITH relation AS (
                SELECT pg_relation_filepath(%s::regclass) AS path,
                       pg_relation_size(%s::regclass) AS relation_size,
                       current_setting('block_size')::integer AS block_size
            ),
            blocks AS (
                SELECT path,
                       block_size,
                       generate_series(
                           0,
                           (relation_size / block_size) - 1
                       ) AS block_no
                FROM relation
            )
            SELECT count(*)
            FROM blocks
            WHERE pg_read_binary_file(
                      path,
                      block_no * block_size,
                      block_size
                  ) = decode(repeat('00', block_size), 'hex')
            """,
            (BINGO_INDEX, BINGO_INDEX),
        )
        zero_pages = cursor.fetchone()[0]
    assert zero_pages == 0


def _top_memory_bytes(connection):
    with connection.cursor() as cursor:
        cursor.execute("""
            SELECT COALESCE(sum(total_bytes), 0)
            FROM pg_backend_memory_contexts
            WHERE name = 'TopMemoryContext'
            """)
        return cursor.fetchone()[0]


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


def test_non_hot_update_vacuum_and_reindex_preserve_bingo_results(
    postgres_storage,
):
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
        cursor.execute(f"""
            INSERT INTO {TABLE} (smiles_isomeric, smiles_indigo)
            VALUES ('CCCCCCCCCCCCCCCCCCCC', 'CCCCCCCCCCCCCCCCCCCC')
            """)

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
                cursor.execute(f"""
                    INSERT INTO {TABLE} (smiles_isomeric, smiles_indigo)
                    SELECT repeat('C', ((g + {worker}) % 48) + 1),
                           repeat('C', ((g + {worker}) % 48) + 1)
                    FROM generate_series(1, {rows_per_worker}) AS g
                    """)
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
                cursor.execute(f"""
                    UPDATE {TABLE}
                    SET smiles_indigo = smiles_indigo || 'C'
                    WHERE (id % {workers}) = {worker}
                    """)
        finally:
            worker_connection.close()

    with ThreadPoolExecutor(max_workers=workers) as executor:
        list(executor.map(update_rows, range(workers)))

    with connection.cursor() as cursor:
        cursor.execute(f"VACUUM (INDEX_CLEANUP ON) {TABLE}")

    _assert_bingo_matches_heap(connection)


@pytest.mark.parametrize("nthreads", [-1, 2])
def test_parallel_build_skips_invalid_structures_and_finishes_cleanly(
    postgres_storage,
    nthreads,
):
    connection, _ = postgres_storage
    _reset_schema(connection, rows=2505, create_bingo_index=False)

    # MAX_CACHE_SIZE is 1000. Put malformed structures around flush boundaries
    # and in the final partial batch so rejection and finalization are both
    # exercised by the parallel build path.
    invalid_ids = (1000, 1001, 2001, 2505)
    with connection.cursor() as cursor:
        cursor.execute(
            f"""
            UPDATE {TABLE}
            SET smiles_isomeric = %s,
                smiles_indigo = %s
            WHERE id = ANY(%s)
            """,
            (INVALID_SMILES, INVALID_SMILES, list(invalid_ids)),
        )

    _create_bingo_index(
        connection,
        nthreads=nthreads,
        reject_invalid_structures=0,
    )

    _assert_bingo_matches_indexable_heap(connection)
    _assert_no_zero_pages(connection)

    shadow_before_invalid = _shadow_counts(connection)
    assert shadow_before_invalid[0] == 2505 - len(invalid_ids)

    with connection.cursor() as cursor:
        cursor.execute(
            f"""
            INSERT INTO {TABLE} (smiles_isomeric, smiles_indigo)
            VALUES (%s, %s)
            """,
            (INVALID_SMILES, INVALID_SMILES),
        )

    assert _shadow_counts(connection) == shadow_before_invalid

    with connection.cursor() as cursor:
        cursor.execute(
            f"""
            INSERT INTO {TABLE} (smiles_isomeric, smiles_indigo)
            VALUES (%s, %s)
            """,
            (VALID_EXACT_SMILES, VALID_EXACT_SMILES),
        )

    shadow_after_valid = _shadow_counts(connection)
    assert shadow_after_valid[0] == shadow_before_invalid[0] + 1
    assert shadow_after_valid[1] > shadow_before_invalid[1]
    _assert_exact_count(connection, VALID_EXACT_SMILES, 1)
    _assert_no_zero_pages(connection)

    with connection.cursor() as cursor:
        cursor.execute(f"REINDEX INDEX {BINGO_INDEX}")

    _assert_bingo_matches_indexable_heap(connection)
    _assert_exact_count(connection, VALID_EXACT_SMILES, 1)
    _assert_no_zero_pages(connection)


def test_reject_invalid_structures_aborts_build_without_publishing_index(
    postgres_storage,
):
    connection, _ = postgres_storage
    _reset_schema(connection, rows=100, create_bingo_index=False)

    with connection.cursor() as cursor:
        cursor.execute(
            f"""
            UPDATE {TABLE}
            SET smiles_isomeric = %s,
                smiles_indigo = %s
            WHERE id = 50
            """,
            (INVALID_SMILES, INVALID_SMILES),
        )

    with pytest.raises(psycopg2.Error):
        _create_bingo_index(
            connection,
            nthreads=2,
            reject_invalid_structures=1,
        )

    with connection.cursor() as cursor:
        cursor.execute("SELECT to_regclass(%s)", (BINGO_INDEX,))
        assert cursor.fetchone()[0] is None
        cursor.execute("SELECT to_regclass(%s)", (SHADOW_TABLE,))
        assert cursor.fetchone()[0] is None
        cursor.execute("SELECT to_regclass(%s)", (SHADOW_HASH_TABLE,))
        assert cursor.fetchone()[0] is None
        cursor.execute(f"""
            UPDATE {TABLE}
            SET smiles_isomeric = 'CC',
                smiles_indigo = 'CC'
            WHERE id = 50
            """)

    _create_bingo_index(
        connection,
        nthreads=2,
        reject_invalid_structures=1,
    )
    _assert_bingo_matches_heap(connection)
    _assert_no_zero_pages(connection)

    shadow_before_invalid = _shadow_counts(connection)
    with pytest.raises(psycopg2.Error):
        with connection.cursor() as cursor:
            cursor.execute(
                f"""
                INSERT INTO {TABLE} (smiles_isomeric, smiles_indigo)
                VALUES (%s, %s)
                """,
                (INVALID_SMILES, INVALID_SMILES),
            )

    assert _shadow_counts(connection) == shadow_before_invalid

    with connection.cursor() as cursor:
        cursor.execute(
            f"""
            SELECT count(*)
            FROM {TABLE}
            WHERE smiles_isomeric = %s
            """,
            (INVALID_SMILES,),
        )
        assert cursor.fetchone()[0] == 0
        cursor.execute(
            f"""
            INSERT INTO {TABLE} (smiles_isomeric, smiles_indigo)
            VALUES (%s, %s)
            """,
            (VALID_EXACT_SMILES, VALID_EXACT_SMILES),
        )

    shadow_after_valid = _shadow_counts(connection)
    assert shadow_after_valid[0] == shadow_before_invalid[0] + 1
    assert shadow_after_valid[1] > shadow_before_invalid[1]
    _assert_exact_count(connection, VALID_EXACT_SMILES, 1)
    _assert_no_zero_pages(connection)


def test_repeated_checkmolecule_does_not_grow_top_memory_context(
    postgres_storage,
):
    connection, _ = postgres_storage

    with connection.cursor() as cursor:
        cursor.execute("""
            SELECT count(*)
            FROM generate_series(1, 10) AS g
            WHERE bingo.checkmolecule(
                CASE WHEN g % 2 = 0 THEN 'CC' ELSE 'CCC' END
            ) IS NULL
            """)
        assert cursor.fetchone()[0] == 10

    before = _top_memory_bytes(connection)

    with connection.cursor() as cursor:
        cursor.execute("""
            SELECT count(*)
            FROM generate_series(1, 50000) AS g
            WHERE bingo.checkmolecule(
                CASE WHEN g % 2 = 0 THEN 'CC' ELSE 'CCC' END
            ) IS NULL
            """)
        assert cursor.fetchone()[0] == 50000

    after = _top_memory_bytes(connection)
    assert after - before < 1024 * 1024
