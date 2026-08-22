#!/usr/bin/env bash
set -euo pipefail

# Physical-streaming-replica test for Bingo WAL. The patched Bingo shared
# library must be installed on BOTH primary and standby PostgreSQL servers.
# PRIMARY_DSN and STANDBY_DSN must address the same database in one physical
# replication pair. The standby must allow read-only hot-standby queries.
#
# Usage:
#   PRIMARY_DSN='host=primary dbname=test user=postgres' \
#   STANDBY_DSN='host=standby dbname=test user=postgres' \
#     ./postgres_wal_replica_test.sh

: "${PRIMARY_DSN:?PRIMARY_DSN is required}"
: "${STANDBY_DSN:?STANDBY_DSN is required}"

PSQL=${PSQL:-psql}
WAIT_SECONDS=${WAIT_SECONDS:-60}
ROWS=${ROWS:-6000}
SCHEMA=bingo_wal_replica_regression
INDEX=${SCHEMA}.structures_molecule_bingo

primary_psql() {
    "$PSQL" -X -v ON_ERROR_STOP=1 "$PRIMARY_DSN" "$@"
}

standby_psql() {
    "$PSQL" -X -v ON_ERROR_STOP=1 "$STANDBY_DSN" "$@"
}

primary_scalar() {
    primary_psql -Atq -c "$1"
}

standby_scalar() {
    standby_psql -Atq -c "$1"
}

wait_replay() {
    local target=$1
    local i
    for ((i = 0; i < WAIT_SECONDS; ++i)); do
        local caught_up
        caught_up=$(standby_scalar "SELECT COALESCE(pg_last_wal_replay_lsn() >= '${target}'::pg_lsn, false)") || true
        if [[ "$caught_up" == "t" ]]; then
            return 0
        fi
        sleep 1
    done
    echo "standby did not replay through ${target} within ${WAIT_SECONDS}s" >&2
    exit 1
}

flush_target() {
    primary_scalar "SELECT pg_current_wal_flush_lsn()"
}

assert_same() {
    local description=$1
    local sql=$2
    local primary_value standby_value
    primary_value=$(primary_scalar "$sql")
    standby_value=$(standby_scalar "$sql")
    if [[ "$primary_value" != "$standby_value" ]]; then
        echo "${description} differs: primary=${primary_value} standby=${standby_value}" >&2
        exit 1
    fi
    echo "${description}: ${primary_value}"
}

cleanup() {
    primary_psql -q -c "DROP SCHEMA IF EXISTS ${SCHEMA} CASCADE" >/dev/null 2>&1 || true
}
trap cleanup EXIT

# Keep Bingo's own structure-processing path single-threaded so the replay
# assertions are deterministic and isolate PostgreSQL/WAL behavior.
primary_psql -q -c "UPDATE bingo.bingo_config SET cvalue = '1' WHERE cname = 'NTHREADS'"
nthreads=$(primary_scalar "SELECT cvalue FROM bingo.bingo_config WHERE cname = 'NTHREADS'")
if [[ "$nthreads" != "1" ]]; then
    echo "failed to configure Bingo NTHREADS=1" >&2
    exit 1
fi

# Phase 1: CREATE INDEX / bulk-build WAL replay.
primary_psql <<SQL
DROP SCHEMA IF EXISTS ${SCHEMA} CASCADE;
CREATE SCHEMA ${SCHEMA};

CREATE TABLE ${SCHEMA}.structures (
    id bigint GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    molecule text NOT NULL,
    payload text NOT NULL
);

INSERT INTO ${SCHEMA}.structures (molecule, payload)
SELECT repeat('C', (g % 40) + 1), 'seed-' || g
FROM generate_series(1, ${ROWS}) AS g;

CREATE INDEX structures_payload_idx
ON ${SCHEMA}.structures (payload);

CREATE INDEX structures_molecule_bingo
ON ${SCHEMA}.structures
USING bingo_idx (molecule bingo.molecule);
SQL

target=$(flush_target)
wait_replay "$target"

assert_same "bulk-build Bingo substructure count" \
    "SELECT count(*) FROM ${SCHEMA}.structures WHERE molecule @ ('CC', '')::bingo.sub"
assert_same "bulk-build index size" \
    "SELECT pg_relation_size('${INDEX}')"

# Phase 2: normal inserts, unrelated indexed-column updates, and VACUUM removal
# must all replay through the custom Bingo relation.
primary_psql <<SQL
INSERT INTO ${SCHEMA}.structures (molecule, payload)
SELECT repeat('C', (g % 50) + 2), 'incremental-' || g
FROM generate_series(1, 500) AS g;

UPDATE ${SCHEMA}.structures
SET payload = payload || '-updated'
WHERE id % 7 = 0;

DELETE FROM ${SCHEMA}.structures
WHERE id % 17 = 0;
SQL
primary_psql -q -c "VACUUM (INDEX_CLEANUP ON) ${SCHEMA}.structures"

target=$(flush_target)
wait_replay "$target"

assert_same "incremental Bingo substructure count" \
    "SELECT count(*) FROM ${SCHEMA}.structures WHERE molecule @ ('CC', '')::bingo.sub"
assert_same "incremental exact heap count" \
    "SELECT count(*) FROM ${SCHEMA}.structures"
assert_same "unrelated-column update count" \
    "SELECT count(*) FROM ${SCHEMA}.structures WHERE payload LIKE '%-updated'"

# Prove the standby can execute more than a cached/simple query after replay.
primary_exact=$(primary_scalar "SELECT count(*) FROM ${SCHEMA}.structures WHERE molecule @ ('CCCCCC', '')::bingo.sub")
standby_exact=$(standby_scalar "SELECT count(*) FROM ${SCHEMA}.structures WHERE molecule @ ('CCCCCC', '')::bingo.sub")
if [[ "$primary_exact" != "$standby_exact" ]]; then
    echo "post-replay Bingo query differs: primary=${primary_exact} standby=${standby_exact}" >&2
    exit 1
fi

# Phase 3: REINDEX uses the bulk-build final relation WAL path and swaps the
# index relation. The physical standby must replay the replacement and remain
# queryable without rebuilding locally.
primary_psql -q -c "REINDEX INDEX ${INDEX}"
target=$(flush_target)
wait_replay "$target"

assert_same "post-REINDEX Bingo substructure count" \
    "SELECT count(*) FROM ${SCHEMA}.structures WHERE molecule @ ('CC', '')::bingo.sub"
assert_same "post-REINDEX index size" \
    "SELECT pg_relation_size('${INDEX}')"

echo "Bingo physical-standby WAL replay regression test passed"
