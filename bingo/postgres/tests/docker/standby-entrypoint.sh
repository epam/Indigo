#!/usr/bin/env bash
set -euo pipefail

PRIMARY_HOST=${PRIMARY_HOST:-primary}
PRIMARY_PORT=${PRIMARY_PORT:-5432}
REPLICATION_USER=${REPLICATION_USER:-replicator}
REPLICATION_PASSWORD=${REPLICATION_PASSWORD:-replicator}
PGDATA=${PGDATA:-/var/lib/postgresql/data}
PGPASSFILE=/var/lib/postgresql/.pgpass

mkdir -p "$PGDATA"
chown -R postgres:postgres "$PGDATA"

cat >"$PGPASSFILE" <<EOF
${PRIMARY_HOST}:${PRIMARY_PORT}:*:${REPLICATION_USER}:${REPLICATION_PASSWORD}
EOF
chown postgres:postgres "$PGPASSFILE"
chmod 0600 "$PGPASSFILE"

if [[ ! -s "$PGDATA/PG_VERSION" ]]; then
    echo "Waiting for primary at ${PRIMARY_HOST}:${PRIMARY_PORT}"
    until pg_isready -h "$PRIMARY_HOST" -p "$PRIMARY_PORT" -U postgres -d test >/dev/null 2>&1; do
        sleep 1
    done

    echo "Taking physical base backup from primary"
    rm -rf "${PGDATA:?}"/*
    gosu postgres env PGPASSFILE="$PGPASSFILE" pg_basebackup \
        -h "$PRIMARY_HOST" \
        -p "$PRIMARY_PORT" \
        -U "$REPLICATION_USER" \
        -D "$PGDATA" \
        -Fp \
        -Xs \
        -P \
        -R

    # Make the credential source explicit for subsequent startup rather than
    # relying on the environment that existed only during pg_basebackup.
    cat >>"$PGDATA/postgresql.auto.conf" <<EOF
primary_conninfo = 'host=${PRIMARY_HOST} port=${PRIMARY_PORT} user=${REPLICATION_USER} passfile=''${PGPASSFILE}'''
EOF
    chown postgres:postgres "$PGDATA/postgresql.auto.conf"
fi

exec docker-entrypoint.sh postgres \
    -c hot_standby=on \
    -c fsync=on \
    -c full_page_writes=on
