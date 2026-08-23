#!/usr/bin/env bash
set -euo pipefail

# Runs the repository's native bingo/tests pytest suite against the primary
# database, which initdb has already populated with the Bingo extension.
# bingo/tests resolves db_config.ini and its data/ directory relative to the
# working directory, so the suite is executed from its copied location after
# regenerating db_config.ini with this harness's connection parameters.

TESTS_DIR=/opt/bingo_tests
PYBIN=/opt/bingo-func-venv/bin

PGHOST=${FUNCTIONAL_PGHOST:-primary}
PGPORT=${FUNCTIONAL_PGPORT:-5432}
DATABASE=${POSTGRES_DB:-test}
PGUSER=${FUNCTIONAL_PGUSER:-postgres}
PGPASSWORD=${FUNCTIONAL_PGPASSWORD:-postgres}

cat > "${TESTS_DIR}/db_config.ini" <<EOF
[common]
bingo_schema=bingo
test_schema=test

[postgres]
host=${PGHOST}
port=${PGPORT}
database=${DATABASE}
user=${PGUSER}
password=${PGPASSWORD}

[bingo-nosql]
db_name=bingo_nosql_db
db_dir=../data

[oracle]
host=localhost
port=1521
database=XEPDB1
user=test
password=test

[bingo-elastic]
host=localhost
port=9200
EOF

cd "${TESTS_DIR}"
exec "${PYBIN}/pytest" --db postgres "$@"
