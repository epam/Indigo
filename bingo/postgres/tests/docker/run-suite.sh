#!/usr/bin/env bash
set -euo pipefail

suite=${1:-normal}
POSTGRES_DB=${POSTGRES_DB:-test}
PSQL=${PSQL:-psql}

export PSQL

if [[ -f /opt/bingo-harness/source-describe ]]; then
    echo "Bingo source under test: $(cat /opt/bingo-harness/source-describe)"
fi
if [[ -f /opt/bingo-harness/source-revision ]]; then
    echo "Bingo source revision: $(cat /opt/bingo-harness/source-revision)"
fi
if [[ -f /opt/bingo-harness/libbingo-postgres.sha256 ]]; then
    echo "Bingo library under test:"
    cat /opt/bingo-harness/libbingo-postgres.sha256
fi

case "$suite" in
    normal)
        exec /opt/bingo-tests/postgres_wal_build_test.sh "$POSTGRES_DB"
        ;;
    concurrent)
        exec env \
            WORKERS="${WORKERS:-8}" \
            ROWS="${ROWS:-6000}" \
            /opt/bingo-tests/postgres_concurrent_update_test.sh "$POSTGRES_DB"
        ;;
    replica)
        : "${PRIMARY_DSN:?PRIMARY_DSN is required for replica suite}"
        : "${STANDBY_DSN:?STANDBY_DSN is required for replica suite}"
        exec env \
            PRIMARY_DSN="$PRIMARY_DSN" \
            STANDBY_DSN="$STANDBY_DSN" \
            WAIT_SECONDS="${WAIT_SECONDS:-120}" \
            ROWS="${ROWS:-6000}" \
            /opt/bingo-tests/postgres_wal_replica_test.sh
        ;;
    *)
        echo "Unknown test suite: $suite" >&2
        echo "Expected one of: normal, concurrent, replica" >&2
        exit 2
        ;;
esac
