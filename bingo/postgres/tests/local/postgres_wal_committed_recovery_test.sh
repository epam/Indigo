#!/usr/bin/env bash
set -euo pipefail

# Destructive committed-write durability test for a DISPOSABLE PostgreSQL
# instance. It models the production sequence more closely than the in-flight
# crash test: a healthy checkpointed Bingo index receives committed work,
# PostgreSQL is stopped uncleanly, and only then does an unrelated indexed-column
# UPDATE invoke Bingo again with an unchanged molecule key/new heap TID.

if [[ "${BINGO_WAL_TEST_ALLOW_CRASH:-}" != "1" ]]; then
    echo "Refusing to crash PostgreSQL. Set BINGO_WAL_TEST_ALLOW_CRASH=1 for a disposable cluster." >&2
    exit 2
fi

: "${PGDATA:?PGDATA must point to the disposable PostgreSQL cluster to crash}"

PSQL=${PSQL:-psql}
PG_CTL=${PG_CTL:-pg_ctl}
DATABASE=${1:-test}
ROWS=${ROWS:-50000}
COMMITTED_ROWS=${COMMITTED_ROWS:-50000}
SCHEMA=bingo_wal_committed_recovery
INDEX=${SCHEMA}.structures_molecule_bingo
PG_LOG=${PG_LOG:-${PGDATA}/bingo-wal-committed-recovery-test.log}
PRODUCTION_UPDATE_VALUE='O=C1c2ccccc2N=NN1COc1cc(Cl)c(F)cc1'

run_psql() {
    "$PSQL" -X -v ON_ERROR_STOP=1 "$DATABASE" "$@"
}

scalar() {
    run_psql -Atq -c "$1"
}

restart_server() {
    "$PG_CTL" -D "$PGDATA" -w -l "$PG_LOG" start
    run_psql -q -c "SELECT 1" >/dev/null
}

immediate_restart() {
    local phase=$1
    echo "Forcing immediate PostgreSQL stop after committed ${phase}"
    "$PG_CTL" -D "$PGDATA" -m immediate -w stop
    restart_server
}

assert_bingo_matches_heap() {
    local phase=$1
    local expected actual
    expected=$(scalar "SELECT count(*) FROM ${SCHEMA}.structures WHERE length(smiles_isomeric) >= 2")
    actual=$(scalar "SELECT count(*) FROM ${SCHEMA}.structures WHERE smiles_isomeric @ ('CC', '')::bingo.sub")
    if [[ "$actual" != "$expected" ]]; then
        echo "Bingo result mismatch ${phase}: got ${actual}, expected ${expected}" >&2
        exit 1
    fi
}

assert_no_zero_tail() {
    local phase=$1
    local zero_tail_pages
    zero_tail_pages=$(scalar "
WITH s AS (
    SELECT pg_relation_size('${INDEX}') / current_setting('block_size')::bigint AS blocks
), h AS (
    SELECT blk,
           (page_header(get_raw_page('${INDEX}', blk))).*
    FROM s,
         LATERAL generate_series(GREATEST(0::bigint, blocks - 12), blocks - 1) AS blk
)
SELECT count(*)
FROM h
WHERE pagesize = 0 OR lower = 0 OR upper = 0")
    if [[ "$zero_tail_pages" != "0" ]]; then
        echo "found ${zero_tail_pages} zero/uninitialized Bingo tail page(s) ${phase}" >&2
        exit 1
    fi
}

cleanup() {
    if "$PG_CTL" -D "$PGDATA" status >/dev/null 2>&1; then
        run_psql -q -c "DROP SCHEMA IF EXISTS ${SCHEMA} CASCADE" >/dev/null 2>&1 || true
    fi
}
trap cleanup EXIT

run_psql -q -c "CREATE EXTENSION IF NOT EXISTS pageinspect"
run_psql -q -c "UPDATE bingo.bingo_config SET cvalue = '1' WHERE cname = 'NTHREADS'"
nthreads=$(scalar "SELECT cvalue FROM bingo.bingo_config WHERE cname = 'NTHREADS'")
if [[ "$nthreads" != "1" ]]; then
    echo "failed to configure Bingo NTHREADS=1" >&2
    exit 1
fi

# Establish a known-good durable starting point with the same index relationship
# as production: Bingo indexes smiles_isomeric while a normal B-tree indexes
# smiles_indigo. Updating only smiles_indigo is therefore non-HOT while the
# Bingo key remains unchanged.
run_psql <<SQL
DROP SCHEMA IF EXISTS ${SCHEMA} CASCADE;
CREATE SCHEMA ${SCHEMA};

CREATE TABLE ${SCHEMA}.structures (
    id bigint GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    smiles_isomeric text NOT NULL,
    smiles_indigo text NOT NULL
);

INSERT INTO ${SCHEMA}.structures (smiles_isomeric, smiles_indigo)
SELECT repeat('C', (g % 48) + 1), repeat('C', (g % 48) + 1)
FROM generate_series(1, ${ROWS}) AS g;

CREATE INDEX structures_smiles_indigo_idx
ON ${SCHEMA}.structures (smiles_indigo);

CREATE INDEX structures_molecule_bingo
ON ${SCHEMA}.structures
USING bingo_idx (smiles_isomeric bingo.molecule);

CHECKPOINT;
SQL

assert_bingo_matches_heap "before committed extension"

# Phase 1: commit a workload that extends/mutates Bingo, then immediately lose
# shared buffers. The heap rows are committed and must remain represented by the
# Bingo index after PostgreSQL WAL recovery.
run_psql -q -c "
INSERT INTO ${SCHEMA}.structures (smiles_isomeric, smiles_indigo)
SELECT repeat('C', (g % 64) + 1), repeat('C', (g % 64) + 1)
FROM generate_series(1, ${COMMITTED_ROWS}) AS g"

committed_visible=$(scalar "SELECT count(*) FROM ${SCHEMA}.structures WHERE id > ${ROWS}")
if [[ "$committed_visible" != "$COMMITTED_ROWS" ]]; then
    echo "committed workload row count is ${committed_visible}, expected ${COMMITTED_ROWS}" >&2
    exit 1
fi

immediate_restart "Bingo incremental INSERT workload"

committed_visible=$(scalar "SELECT count(*) FROM ${SCHEMA}.structures WHERE id > ${ROWS}")
if [[ "$committed_visible" != "$COMMITTED_ROWS" ]]; then
    echo "committed heap rows missing after recovery: got ${committed_visible}, expected ${COMMITTED_ROWS}" >&2
    exit 1
fi

# Phase 2: make the production-shaped statement the first Bingo access after
# the unclean restart. This mirrors the incident: the UPDATE did not create the
# earlier damage; its new heap TID caused Bingo to encounter that damage later.
# Only smiles_indigo changes, but because it has a B-tree index this is non-HOT;
# PostgreSQL creates a new heap TID and calls Bingo aminsert with the unchanged
# smiles_isomeric key. The value is the same one present in the production stack
# trace.
target_id=$(scalar "SELECT min(id) FROM ${SCHEMA}.structures")
run_psql -q -c "
UPDATE ${SCHEMA}.structures
SET smiles_indigo = '${PRODUCTION_UPDATE_VALUE}'
WHERE id = ${target_id}"

updated=$(scalar "SELECT count(*) FROM ${SCHEMA}.structures WHERE id = ${target_id} AND smiles_indigo = '${PRODUCTION_UPDATE_VALUE}'")
if [[ "$updated" != "1" ]]; then
    echo "production-shaped non-HOT UPDATE did not commit" >&2
    exit 1
fi
assert_bingo_matches_heap "after production-shaped non-HOT UPDATE"

# Prove the index entry written for that new heap TID is itself recoverable.
immediate_restart "production-shaped non-HOT UPDATE"

updated=$(scalar "SELECT count(*) FROM ${SCHEMA}.structures WHERE id = ${target_id} AND smiles_indigo = '${PRODUCTION_UPDATE_VALUE}'")
if [[ "$updated" != "1" ]]; then
    echo "committed production-shaped UPDATE missing after recovery" >&2
    exit 1
fi
assert_bingo_matches_heap "after non-HOT UPDATE recovery"

run_psql -q -c "CHECKPOINT"
assert_no_zero_tail "after non-HOT UPDATE recovery"

# Recovery must leave the relation writable as well as searchable.
run_psql -q -c "
INSERT INTO ${SCHEMA}.structures (smiles_isomeric, smiles_indigo)
VALUES ('CCCCCCCCCCCCCCCCCCCC', 'CCCCCCCCCCCCCCCCCCCC')"
assert_bingo_matches_heap "after post-recovery insert"

echo "Bingo committed incremental/non-HOT recovery test passed"
