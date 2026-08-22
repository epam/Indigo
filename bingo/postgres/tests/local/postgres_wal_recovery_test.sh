#!/usr/bin/env bash
set -euo pipefail

# Destructive crash/recovery test for a DISPOSABLE PostgreSQL instance.
# It intentionally uses `pg_ctl stop -m immediate` while Bingo is mutating its
# index. Never point PGDATA at a production or otherwise valuable cluster.
#
# Usage:
#   BINGO_WAL_TEST_ALLOW_CRASH=1 \
#   PGDATA=/path/to/disposable/data \
#   PSQL=/usr/local/pgsql/bin/psql \
#   PG_CTL=/usr/local/pgsql/bin/pg_ctl \
#     ./postgres_wal_recovery_test.sh [database]

if [[ "${BINGO_WAL_TEST_ALLOW_CRASH:-}" != "1" ]]; then
    echo "Refusing to crash PostgreSQL. Set BINGO_WAL_TEST_ALLOW_CRASH=1 for a disposable cluster." >&2
    exit 2
fi

: "${PGDATA:?PGDATA must point to the disposable PostgreSQL cluster to crash}"

PSQL=${PSQL:-psql}
PG_CTL=${PG_CTL:-pg_ctl}
DATABASE=${1:-test}
ROWS=${ROWS:-50000}
CRASH_ROWS=${CRASH_ROWS:-150000}
CRASH_DELAY=${CRASH_DELAY:-0.20}
ORPHAN_GROW_ROWS=${ORPHAN_GROW_ROWS:-20000}
SCHEMA=bingo_wal_crash_regression
INDEX=${SCHEMA}.structures_molecule_bingo
PG_LOG=${PG_LOG:-${PGDATA}/bingo-wal-recovery-test.log}

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

crash_writer() {
    local writer_pid=$1
    local phase=$2

    sleep "$CRASH_DELAY"
    if ! kill -0 "$writer_pid" 2>/dev/null; then
        wait "$writer_pid" || true
        echo "${phase} finished before the crash point; increase ROWS/CRASH_ROWS or reduce CRASH_DELAY" >&2
        exit 1
    fi

    echo "Forcing immediate PostgreSQL stop during ${phase}"
    "$PG_CTL" -D "$PGDATA" -m immediate -w stop
    wait "$writer_pid" || true
    restart_server
}

cleanup() {
    if "$PG_CTL" -D "$PGDATA" status >/dev/null 2>&1; then
        run_psql -q -c "DROP SCHEMA IF EXISTS ${SCHEMA} CASCADE" >/dev/null 2>&1 || true
    fi
}
trap cleanup EXIT

run_psql -q -c "CREATE EXTENSION IF NOT EXISTS pageinspect"

# Keep Bingo's own structure-processing path single-threaded so crash timing
# tests PostgreSQL/WAL durability without also varying Bingo worker scheduling.
run_psql -q -c "UPDATE bingo.bingo_config SET cvalue = '1' WHERE cname = 'NTHREADS'"
nthreads=$(scalar "SELECT cvalue FROM bingo.bingo_config WHERE cname = 'NTHREADS'")
if [[ "$nthreads" != "1" ]]; then
    echo "failed to configure Bingo NTHREADS=1" >&2
    exit 1
fi

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

CREATE INDEX structures_molecule_bingo
ON ${SCHEMA}.structures
USING bingo_idx (molecule bingo.molecule);

CHECKPOINT;
SQL

seed_matches=$(scalar "SELECT count(*) FROM ${SCHEMA}.structures WHERE molecule @ ('CC', '')::bingo.sub")
if (( seed_matches <= 0 )); then
    echo "baseline Bingo query failed" >&2
    exit 1
fi

# Phase 1: crash while extending/mutating Bingo for direct inserts. The entire
# SQL statement is one transaction, so recovery should make these heap rows
# invisible even though index WAL may contain harmless dead entries/holes.
(
    run_psql -q -c "
        INSERT INTO ${SCHEMA}.structures (molecule, payload)
        SELECT repeat('C', (g % 64) + 1), 'crash-insert-' || g
        FROM generate_series(1, ${CRASH_ROWS}) AS g;
    "
) >"${PGDATA}/bingo-crash-insert-client.log" 2>&1 &
insert_pid=$!
crash_writer "$insert_pid" "Bingo INSERT"

visible_crash_inserts=$(scalar "SELECT count(*) FROM ${SCHEMA}.structures WHERE payload LIKE 'crash-insert-%'")
if [[ "$visible_crash_inserts" != "0" ]]; then
    echo "uncommitted crash INSERT rows became visible after recovery: ${visible_crash_inserts}" >&2
    exit 1
fi

run_psql <<SQL
SELECT count(*) AS matches_after_insert_crash
FROM ${SCHEMA}.structures
WHERE molecule @ ('CC', '')::bingo.sub;

INSERT INTO ${SCHEMA}.structures (molecule, payload)
VALUES ('CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC', 'post-insert-crash');
SQL

post_insert_visible=$(scalar "SELECT count(*) FROM ${SCHEMA}.structures WHERE payload = 'post-insert-crash'")
if [[ "$post_insert_visible" != "1" ]]; then
    echo "Bingo could not accept a new row after INSERT crash recovery" >&2
    exit 1
fi

# Phase 2: crash during non-key updates. payload is B-tree indexed, so these
# are non-HOT updates and PostgreSQL invokes Bingo for a new heap TID even
# though the molecule itself is unchanged.
(
    run_psql -q -c "
        UPDATE ${SCHEMA}.structures
        SET payload = payload || '-crash-update-' || id
        WHERE id <= ${ROWS};
    "
) >"${PGDATA}/bingo-crash-update-client.log" 2>&1 &
update_pid=$!
crash_writer "$update_pid" "non-key UPDATE invoking Bingo"

visible_crash_updates=$(scalar "SELECT count(*) FROM ${SCHEMA}.structures WHERE payload LIKE '%-crash-update-%'")
if [[ "$visible_crash_updates" != "0" ]]; then
    echo "uncommitted crash UPDATE rows became visible after recovery: ${visible_crash_updates}" >&2
    exit 1
fi

run_psql <<SQL
SELECT count(*) AS matches_after_update_crash
FROM ${SCHEMA}.structures
WHERE molecule @ ('CC', '')::bingo.sub;

INSERT INTO ${SCHEMA}.structures (molecule, payload)
VALUES ('CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC', 'post-update-crash');
SQL

# Phase 3: exercise VACUUM/Bingo removals, and crash it if the workload is long
# enough to still be active at the configured delay. A completed VACUUM is not
# an error; the prior phases already guarantee an actual immediate-stop crash.
run_psql -q -c "DELETE FROM ${SCHEMA}.structures WHERE id <= ${ROWS} AND id % 2 = 0"
(
    run_psql -q -c "VACUUM (INDEX_CLEANUP ON) ${SCHEMA}.structures"
) >"${PGDATA}/bingo-crash-vacuum-client.log" 2>&1 &
vacuum_pid=$!
sleep "$CRASH_DELAY"
if kill -0 "$vacuum_pid" 2>/dev/null; then
    echo "Forcing immediate PostgreSQL stop during Bingo VACUUM cleanup"
    "$PG_CTL" -D "$PGDATA" -m immediate -w stop
    wait "$vacuum_pid" || true
    restart_server
else
    wait "$vacuum_pid"
    echo "VACUUM completed before crash point; continuing with structural validation"
fi

# A second VACUUM must be able to complete after recovery regardless of where
# the crashed cleanup stopped.
run_psql -q -c "VACUUM (INDEX_CLEANUP ON) ${SCHEMA}.structures"
run_psql -q -c "CHECKPOINT"

# Every stored molecule is a run of 'C' characters, so every row whose
# molecule has at least two carbons must be returned by the substructure
# search. Compare against that independent oracle instead of only requiring a
# non-zero result.
expected_matches=$(scalar "SELECT count(*) FROM ${SCHEMA}.structures WHERE length(molecule) >= 2")
post_vacuum_matches=$(scalar "SELECT count(*) FROM ${SCHEMA}.structures WHERE molecule @ ('CC', '')::bingo.sub")
if [[ "$post_vacuum_matches" != "$expected_matches" ]]; then
    echo "Bingo search mismatch after VACUUM crash/recovery: got ${post_vacuum_matches}, expected ${expected_matches}" >&2
    exit 1
fi

# A crashed tail extension can leave an unreferenced zero-filled block exactly
# at EOF. Bingo skips such blocks during scans and reuses them the next time
# the index extends, so their immediate presence is not corruption. Corruption
# would be: search results diverging from the oracle (checked above), or zero
# blocks still present after the index was forced to grow over them.
zero_tail_pages() {
    scalar "
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
WHERE pagesize = 0 OR lower = 0 OR upper = 0"
}

orphans=$(zero_tail_pages)
if [[ "$orphans" != "0" ]]; then
    echo "note: ${orphans} orphaned zero tail block(s) after recovery; forcing index growth to validate reuse" >&2
    run_psql -q -c "
        INSERT INTO ${SCHEMA}.structures (molecule, payload)
        SELECT repeat('C', (g % 64) + 1), 'orphan-reuse-' || g
        FROM generate_series(1, ${ORPHAN_GROW_ROWS}) AS g"

    remaining=$(zero_tail_pages)
    if [[ "$remaining" != "0" ]]; then
        echo "found ${remaining} zero/uninitialized Bingo tail page(s) even after forced index growth" >&2
        exit 1
    fi

    expected_matches=$(scalar "SELECT count(*) FROM ${SCHEMA}.structures WHERE length(molecule) >= 2")
    post_growth_matches=$(scalar "SELECT count(*) FROM ${SCHEMA}.structures WHERE molecule @ ('CC', '')::bingo.sub")
    if [[ "$post_growth_matches" != "$expected_matches" ]]; then
        echo "Bingo search mismatch after orphan reuse: got ${post_growth_matches}, expected ${expected_matches}" >&2
        exit 1
    fi
fi

run_psql <<SQL
WITH s AS (
    SELECT pg_relation_size('${INDEX}') / current_setting('block_size')::bigint AS blocks
)
SELECT blk, lsn, lower, upper, special, pagesize, version
FROM s,
     LATERAL generate_series(GREATEST(0::bigint, blocks - 6), blocks - 1) AS blk,
     LATERAL page_header(get_raw_page('${INDEX}', blk));
SQL

echo "Bingo immediate-crash WAL recovery regression test passed"
