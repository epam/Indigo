#!/usr/bin/env bash
set -euo pipefail

# This runs only when the disposable primary initializes a new PGDATA.
# The credentials are intentionally test-only and scoped to the Compose network.
psql -v ON_ERROR_STOP=1 \
    --username "$POSTGRES_USER" \
    --dbname "$POSTGRES_DB" <<'SQL'
DO $$
BEGIN
    IF NOT EXISTS (SELECT 1 FROM pg_roles WHERE rolname = 'replicator') THEN
        CREATE ROLE replicator WITH REPLICATION LOGIN PASSWORD 'replicator';
    END IF;
END
$$;
SQL

cat >>"$PGDATA/pg_hba.conf" <<'HBA'
host replication replicator 0.0.0.0/0 scram-sha-256
host replication replicator ::/0 scram-sha-256
HBA
