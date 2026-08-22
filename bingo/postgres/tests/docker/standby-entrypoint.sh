#!/usr/bin/env bash
set -euo pipefail

PRIMARY_HOST=${PRIMARY_HOST:-primary}
PRIMARY_PORT=${PRIMARY_PORT:-5432}
REPLICATION_USER=${REPLICATION_USER:-replicator}
REPLICATION_PASSWORD=${REPLICATION_PASSWORD:-replicator}
PGDATA=${PGDATA:-/var/lib/postgresql/data}

mkdir -p "$PGDATA"
chown -R postgres:postgres "$PGDATA"

if [[ ! -s "$PGDATA/PG_VERSION" ]]; then
    echo "Waiting for primary at ${PRIMARY_HOST}:${PRIMARY_PORT}"
    until pg_isready -h "$PRIMARY_HOST" -p "$PRIMARY_PORT" -U postgres -d test >/dev/null 2>&1; do
        sleep 1
    done

    echo "Taking physical base backup from primary"
    rm -rf "${PGDATA:?}"/*
    export PGPASSWORD="$REPLICATION_PASSWORD"
    gosu postgres pg_basebackup \
        -h "$PRIMARY_HOST" \
        -p "$PRIMARY_PORT" \
        -U "$REPLICATION_USER" \
        -D "$PGDATA" \
        -Fp \
        -Xs \
        -P \
        -R
    unset PGPASSWORD
fi

exec docker-entrypoint.sh postgres \
    -c hot_standby=on \
    -c fsync=on \
    -c full_page_writes=on
