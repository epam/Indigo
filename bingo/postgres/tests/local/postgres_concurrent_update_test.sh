#!/usr/bin/env bash
set -euo pipefail

# Regression test for concurrent Bingo mutations. It specifically exercises the
# case where PostgreSQL updates a non-Bingo indexed column, creating a new heap
# TID and therefore invoking bingo_insert() with an unchanged molecule key.
#
# Usage:
#   PSQL=/usr/local/pgsql/bin/psql ./postgres_concurrent_update_test.sh [database]
#
# The Bingo SQL objects must already be installed in the target database.

PSQL=${PSQL:-psql}
DATABASE=${1:-test}
WORKERS=${WORKERS:-8}
ROWS=${ROWS:-4000}
SCHEMA=bingo_concurrency_regression

run_psql() {
    "$PSQL" -X -v ON_ERROR_STOP=1 "$DATABASE" "$@"
}

cleanup() {
    run_psql -q -c "DROP SCHEMA IF EXISTS ${SCHEMA} CASCADE" >/dev/null 2>&1 || true
}
trap cleanup EXIT

run_psql <<SQL
DROP SCHEMA IF EXISTS ${SCHEMA} CASCADE;
CREATE SCHEMA ${SCHEMA};

CREATE TABLE ${SCHEMA}.structures (
    id bigint GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    molecule text NOT NULL,
    payload text NOT NULL
);

INSERT INTO ${SCHEMA}.structures (molecule, payload)
SELECT repeat('C', (g % 32) + 1), 'initial'
FROM generate_series(1, ${ROWS}) AS g;

CREATE INDEX structures_molecule_bingo
ON ${SCHEMA}.structures
USING bingo_idx (molecule bingo.molecule)
WITH (NTHREADS=1);

-- Updating this column makes the heap update non-HOT while leaving the Bingo
-- key unchanged. PostgreSQL must still insert a new index TID for each row.
CREATE INDEX structures_payload_idx
ON ${SCHEMA}.structures (payload);
SQL

pids=()
for worker in $(seq 0 $((WORKERS - 1))); do
    run_psql -q -c "
        UPDATE ${SCHEMA}.structures
        SET payload = payload || '-w${worker}'
        WHERE (id % ${WORKERS}) = ${worker};
    " &
    pids+=("$!")
done

failed=0
for pid in "${pids[@]}"; do
    if ! wait "$pid"; then
        failed=1
    fi
done

if (( failed != 0 )); then
    echo "concurrent non-key updates failed" >&2
    exit 1
fi

# Exercise direct concurrent inserts too; this is the minimal shape of the
# long-standing upstream concurrent Bingo insertion corruption report.
pids=()
for worker in $(seq 0 $((WORKERS - 1))); do
    run_psql -q -c "
        INSERT INTO ${SCHEMA}.structures (molecule, payload)
        SELECT repeat('C', ((g + ${worker}) % 32) + 1), 'insert-w${worker}'
        FROM generate_series(1, 250) AS g;
    " &
    pids+=("$!")
done

failed=0
for pid in "${pids[@]}"; do
    if ! wait "$pid"; then
        failed=1
    fi
done

if (( failed != 0 )); then
    echo "concurrent inserts failed" >&2
    exit 1
fi

run_psql <<SQL
SELECT count(*) AS total_rows
FROM ${SCHEMA}.structures;

-- Force a Bingo index scan after the concurrent mutations so corruption is
-- detected by the same read path that production queries use.
SELECT count(*) AS carbon_chain_matches
FROM ${SCHEMA}.structures
WHERE molecule @ ('CC', '')::bingo.sub;
SQL

echo "Bingo concurrent mutation regression test passed"
