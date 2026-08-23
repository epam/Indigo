#!/usr/bin/env bash
set -euo pipefail

# Destructive CREATE INDEX durability test for a DISPOSABLE PostgreSQL instance.
# The test commits a Bingo index build, stops PostgreSQL immediately before any
# explicit checkpoint, restarts it, and verifies the committed index from heap
# truth. Stock Bingo 1.45 has no WAL for its custom relation pages; the patched
# build must recover them correctly.

if [[ "${BINGO_WAL_TEST_ALLOW_CRASH:-}" != "1" ]]; then
    echo "Refusing to crash PostgreSQL. Set BINGO_WAL_TEST_ALLOW_CRASH=1 for a disposable cluster." >&2
    exit 2
fi

: "${PGDATA:?PGDATA must point to the disposable PostgreSQL cluster to crash}"

PSQL=${PSQL:-psql}
PG_CTL=${PG_CTL:-pg_ctl}
DATABASE=${1:-test}
ROWS=${ROWS:-50000}
SCHEMA=bingo_wal_build_recovery
INDEX=${SCHEMA}.structures_molecule_bingo
PG_LOG=${PG_LOG:-${PGDATA}/bingo-wal-build-recovery-test.log}

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

# Make the heap and unrelated B-tree durable first. The only work after this
# checkpoint is CREATE INDEX, so recovery failures are attributable to Bingo's
# build relation rather than to the seed table.
run_psql <<SQL
DROP SCHEMA IF EXISTS ${SCHEMA} CASCADE;
CREATE SCHEMA ${SCHEMA};

CREATE TABLE ${SCHEMA}.structures (
    id bigint GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    molecule text NOT NULL,
    payload text NOT NULL
);

INSERT INTO ${SCHEMA}.structures (molecule, payload)
SELECT repeat('C', (g % 48) + 1), 'seed-' || g
FROM generate_series(1, ${ROWS}) AS g;

CREATE INDEX structures_payload_idx
ON ${SCHEMA}.structures (payload);

CHECKPOINT;
SQL

# CREATE INDEX is committed before psql returns. Deliberately do not issue a
# checkpoint afterwards: PostgreSQL must be able to reconstruct the committed
# Bingo relation from WAL after the immediate stop.
run_psql -q -c "
CREATE INDEX structures_molecule_bingo
ON ${SCHEMA}.structures
USING bingo_idx (molecule bingo.molecule)"

immediate_restart "Bingo CREATE INDEX"

expected=$(scalar "SELECT count(*) FROM ${SCHEMA}.structures WHERE length(molecule) >= 2")
actual=$(scalar "SELECT count(*) FROM ${SCHEMA}.structures WHERE molecule @ ('CC', '')::bingo.sub")
if [[ "$actual" != "$expected" ]]; then
    echo "Bingo result mismatch after CREATE INDEX recovery: got ${actual}, expected ${expected}" >&2
    exit 1
fi

# A committed build has no unpublished tail allocation. After recovery and a
# checkpoint every block at the end of the published relation must have a valid
# PostgreSQL page header. This checks the zero-page signature from production.
run_psql -q -c "CHECKPOINT"
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
    echo "found ${zero_tail_pages} zero/uninitialized Bingo tail page(s) after committed CREATE INDEX recovery" >&2
    exit 1
fi

# Recovery must leave the index writable, not merely readable.
run_psql -q -c "
INSERT INTO ${SCHEMA}.structures (molecule, payload)
VALUES ('CCCCCCCCCCCCCCCC', 'post-build-recovery')"

expected=$(scalar "SELECT count(*) FROM ${SCHEMA}.structures WHERE length(molecule) >= 2")
actual=$(scalar "SELECT count(*) FROM ${SCHEMA}.structures WHERE molecule @ ('CC', '')::bingo.sub")
if [[ "$actual" != "$expected" ]]; then
    echo "Bingo result mismatch after post-recovery write: got ${actual}, expected ${expected}" >&2
    exit 1
fi

echo "Bingo committed CREATE INDEX recovery test passed"
