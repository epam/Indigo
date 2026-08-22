#!/usr/bin/env bash
set -euo pipefail

PGDATA=${PGDATA:-/var/lib/postgresql/data}
POSTGRES_DB=${POSTGRES_DB:-test}
PG_LOG=${PG_LOG:-${PGDATA}/postgres.log}

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

PGDATA=${PGDATA:-/var/lib/postgresql/data}
mkdir -p "$PGDATA" /var/run/postgresql
chown -R postgres:postgres "$PGDATA" /var/run/postgresql
chmod 3775 /var/run/postgresql
rm -rf "${PGDATA:?}"/*

gosu postgres initdb -D "$PGDATA" -A trust --no-locale >/dev/null
cat >>"$PGDATA/postgresql.conf" <<'CONF'
listen_addresses = '127.0.0.1'
fsync = on
full_page_writes = on
wal_level = replica
max_wal_senders = 4
CONF

shutdown() {
    if gosu postgres pg_ctl -D "$PGDATA" status >/dev/null 2>&1; then
        gosu postgres pg_ctl -D "$PGDATA" -m fast -w stop >/dev/null 2>&1 || true
    fi
}
trap shutdown EXIT

gosu postgres pg_ctl -D "$PGDATA" -w -l "$PG_LOG" start
gosu postgres createdb "$POSTGRES_DB"
gosu postgres psql -X -v ON_ERROR_STOP=1 -d "$POSTGRES_DB" \
    -f /docker-entrypoint-initdb.d/00-bingo-install.sql >/dev/null

# The regression script performs and recovers from pg_ctl -m immediate itself.
gosu postgres env \
    BINGO_WAL_TEST_ALLOW_CRASH=1 \
    PGDATA="$PGDATA" \
    PG_LOG="$PG_LOG" \
    PSQL=psql \
    PG_CTL=pg_ctl \
    ROWS="${ROWS:-50000}" \
    CRASH_ROWS="${CRASH_ROWS:-150000}" \
    CRASH_DELAY="${CRASH_DELAY:-0.20}" \
    /opt/bingo-tests/postgres_wal_recovery_test.sh "$POSTGRES_DB"
