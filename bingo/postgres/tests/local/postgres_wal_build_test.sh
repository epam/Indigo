#!/usr/bin/env bash
set -euo pipefail

# Non-destructive Bingo WAL/integrity regression test for an existing test DB.
# The patched Bingo extension must already be installed.
#
# Usage:
#   PSQL=/usr/local/pgsql/bin/psql ./postgres_wal_build_test.sh [database]

PSQL=${PSQL:-psql}
DATABASE=${1:-test}
ROWS=${ROWS:-6000}
SCHEMA=bingo_wal_build_regression
INDEX=${SCHEMA}.structures_molecule_bingo

run_psql() {
    "$PSQL" -X -v ON_ERROR_STOP=1 "$DATABASE" "$@"
}

scalar() {
    run_psql -Atq -c "$1"
}

cleanup() {
    run_psql -q -c "DROP SCHEMA IF EXISTS ${SCHEMA} CASCADE" >/dev/null 2>&1 || true
}
trap cleanup EXIT

run_psql -q -c "CREATE EXTENSION IF NOT EXISTS pageinspect"

# Keep Bingo's own structure-processing path single-threaded so this test
# isolates PostgreSQL/WAL behavior. Per-index WITH (NTHREADS=...) is broken on
# modern PostgreSQL in upstream Bingo, so use Bingo's installed config table.
run_psql -q -c "UPDATE bingo.bingo_config SET cvalue = '1' WHERE cname = 'NTHREADS'"
nthreads=$(scalar "SELECT cvalue FROM bingo.bingo_config WHERE cname = 'NTHREADS'")
if [[ "$nthreads" != "1" ]]; then
    echo "failed to configure Bingo NTHREADS=1" >&2
    exit 1
fi

wal_before=$(scalar "SELECT pg_current_wal_lsn()")

run_psql <<SQL
DROP SCHEMA IF EXISTS ${SCHEMA} CASCADE;
CREATE SCHEMA ${SCHEMA};

CREATE TABLE ${SCHEMA}.structures (
    id bigint GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    molecule text NOT NULL,
    payload text NOT NULL
);

INSERT INTO ${SCHEMA}.structures (molecule, payload)
SELECT repeat('C', (g % 32) + 1), 'seed-' || g
FROM generate_series(1, ${ROWS}) AS g;

CREATE INDEX structures_payload_idx
ON ${SCHEMA}.structures (payload);

CREATE INDEX structures_molecule_bingo
ON ${SCHEMA}.structures
USING bingo_idx (molecule bingo.molecule);
SQL

wal_after_build=$(scalar "SELECT pg_current_wal_lsn()")
wal_build_bytes=$(scalar "SELECT pg_wal_lsn_diff('${wal_after_build}'::pg_lsn, '${wal_before}'::pg_lsn)::bigint")
echo "CREATE INDEX WAL bytes (includes heap/catalog work): ${wal_build_bytes}"

baseline=$(scalar "SELECT count(*) FROM ${SCHEMA}.structures WHERE molecule @ ('CC', '')::bingo.sub")
if (( baseline <= 0 )); then
    echo "expected Bingo query to return seed rows" >&2
    exit 1
fi

# Exercise transaction abort. Physical index entries may remain as harmless
# dead entries, but no aborted heap tuple may become visible through Bingo.
run_psql <<SQL
BEGIN;
INSERT INTO ${SCHEMA}.structures (molecule, payload)
VALUES ('CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC', 'rolled-back-insert');
UPDATE ${SCHEMA}.structures
SET payload = payload || '-rolled-back-update'
WHERE id <= 100;
ROLLBACK;
SQL

rollback_count=$(scalar "SELECT count(*) FROM ${SCHEMA}.structures WHERE payload = 'rolled-back-insert'")
if [[ "$rollback_count" != "0" ]]; then
    echo "rolled-back tuple became visible" >&2
    exit 1
fi

# Exercise normal incremental insert and the exact non-key-update shape that
# invokes bingo_insert() with an unchanged molecule key.
run_psql <<SQL
INSERT INTO ${SCHEMA}.structures (molecule, payload)
VALUES ('CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC', 'committed-insert');

UPDATE ${SCHEMA}.structures
SET payload = payload || '-updated'
WHERE id % 7 = 0;
SQL

committed_count=$(scalar "SELECT count(*) FROM ${SCHEMA}.structures WHERE payload = 'committed-insert'")
if [[ "$committed_count" != "1" ]]; then
    echo "committed incremental insert is not visible" >&2
    exit 1
fi

# Exercise Bingo bulk-delete/VACUUM mutation WAL.
run_psql -q -c "DELETE FROM ${SCHEMA}.structures WHERE id % 13 = 0"
run_psql -q -c "VACUUM ${SCHEMA}.structures"

post_vacuum=$(scalar "SELECT count(*) FROM ${SCHEMA}.structures WHERE molecule @ ('CC', '')::bingo.sub")
if (( post_vacuum <= 0 || post_vacuum >= baseline + 1 )); then
    echo "unexpected Bingo result count after DELETE/VACUUM: ${post_vacuum}" >&2
    exit 1
fi

# REINDEX exercises the bulk-build final log_newpage_range() path.
wal_before_reindex=$(scalar "SELECT pg_current_wal_lsn()")
run_psql -q -c "REINDEX INDEX ${INDEX}"
wal_after_reindex=$(scalar "SELECT pg_current_wal_lsn()")
wal_reindex_bytes=$(scalar "SELECT pg_wal_lsn_diff('${wal_after_reindex}'::pg_lsn, '${wal_before_reindex}'::pg_lsn)::bigint")
echo "REINDEX WAL bytes: ${wal_reindex_bytes}"

post_reindex=$(scalar "SELECT count(*) FROM ${SCHEMA}.structures WHERE molecule @ ('CC', '')::bingo.sub")
if [[ "$post_reindex" != "$post_vacuum" ]]; then
    echo "Bingo results changed across REINDEX: before=${post_vacuum} after=${post_reindex}" >&2
    exit 1
fi

# Force all dirty pages out, then inspect the physical tail. The regression we
# are protecting against left completely zero pages at EOF (pagesize = 0).
run_psql -q -c "CHECKPOINT"
zero_tail_pages=$(scalar "
WITH s AS (
    SELECT pg_relation_size('${INDEX}') / current_setting('block_size')::bigint AS blocks
), h AS (
    SELECT blk,
           (page_header(get_raw_page('${INDEX}', blk))).*
    FROM s,
         LATERAL generate_series(GREATEST(0::bigint, blocks - 8), blocks - 1) AS blk
)
SELECT count(*)
FROM h
WHERE pagesize = 0 OR lower = 0 OR upper = 0;
")

if [[ "$zero_tail_pages" != "0" ]]; then
    echo "found ${zero_tail_pages} zero/uninitialized page(s) in the Bingo index tail" >&2
    exit 1
fi

run_psql <<SQL
WITH s AS (
    SELECT pg_relation_size('${INDEX}') / current_setting('block_size')::bigint AS blocks
)
SELECT blk, lsn, lower, upper, special, pagesize, version
FROM s,
     LATERAL generate_series(GREATEST(0::bigint, blocks - 4), blocks - 1) AS blk,
     LATERAL page_header(get_raw_page('${INDEX}', blk));
SQL

echo "Bingo WAL build/mutation integrity regression test passed"
