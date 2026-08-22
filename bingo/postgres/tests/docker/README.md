# Bingo PostgreSQL 17 WAL Docker harness

This harness runs the PostgreSQL/Bingo corruption and WAL regression tests against the exact `libbingo-postgres.so` artifact that you intend to deploy.

It does not build a second copy of Bingo from source. Build/package the patched 1.45 library normally, then copy that shared object into this directory:

```bash
cp /path/to/libbingo-postgres.so \
  bingo/postgres/tests/docker/libbingo-postgres.so
```

The shared object is ignored by Git.

The Docker image is based on `postgres:17`, copies the library to:

```text
/usr/lib/postgresql/17/lib/libbingo-postgres.so
```

and generates `bingo_install.sql` from the checked-in PostgreSQL SQL templates with `bingo-pg-install.sh -pglibdir`. Fresh test databases therefore use the same SQL bindings as the source tree and load the supplied library through PostgreSQL `$libdir`.

## Prerequisites

- Docker with the Compose v2 plugin (`docker compose`)
- a PostgreSQL 17 `libbingo-postgres.so` built from this branch

All credentials and databases created here are disposable test data only.

## Quick start

From the repository root:

```bash
bash bingo/postgres/tests/docker/run.sh build
bash bingo/postgres/tests/docker/run.sh normal
bash bingo/postgres/tests/docker/run.sh concurrent
bash bingo/postgres/tests/docker/run.sh crash
bash bingo/postgres/tests/docker/run.sh replica
```

To run the whole suite:

```bash
bash bingo/postgres/tests/docker/run.sh all
```

To remove containers and all harness database volumes:

```bash
bash bingo/postgres/tests/docker/run.sh down
```

## Suites

### `normal`

Runs `postgres_wal_build_test.sh` against a normal PostgreSQL 17 primary and covers:

- initial Bingo `CREATE INDEX`
- transaction rollback
- incremental insert
- a non-key indexed-column update that still invokes Bingo for a new heap TID
- DELETE + VACUUM/Bingo bulk deletion
- `REINDEX`
- result consistency across rebuild
- physical tail-page inspection for zero/uninitialized pages
- basic WAL-volume reporting

```bash
bash bingo/postgres/tests/docker/run.sh normal
```

The primary is exposed on host port `55432` by default. Override it with `BINGO_TEST_PRIMARY_PORT`.

### `concurrent`

Runs the concurrent-writer regression that reproduces the class of failure reported by upstream issue #108 and also mixes in non-key updates.

```bash
WORKERS=12 ROWS=12000 \
  bash bingo/postgres/tests/docker/run.sh concurrent
```

Writes to one Bingo index are expected to serialize in the patched access method; the test verifies correctness rather than parallel write throughput.

### `crash`

Runs `postgres_wal_recovery_test.sh` against a PostgreSQL instance dedicated to the crash test. The test service uses tmpfs for `PGDATA` and deliberately executes:

```text
pg_ctl stop -m immediate
```

while Bingo is mutating the index. The container itself remains alive and restarts PostgreSQL so recovery can be validated.

This does **not** crash the normal Compose primary and does not mount the Docker socket.

```bash
bash bingo/postgres/tests/docker/run.sh crash
```

Stress parameters can be adjusted without changing the Compose file:

```bash
CRASH_ROWS_BASELINE=100000 \
CRASH_ROWS=300000 \
CRASH_DELAY=0.10 \
  bash bingo/postgres/tests/docker/run.sh crash
```

The crash suite verifies, among other things, that aborted heap rows do not become visible through Bingo, searches and new writes work after recovery, VACUUM can complete after recovery, and the Bingo relation does not end with the zero-page signature seen in the original failure.

### `replica`

Starts a fresh primary and physical PostgreSQL standby using `pg_basebackup -R`. The patched Bingo library is present on both servers.

```bash
bash bingo/postgres/tests/docker/run.sh replica
```

The replica suite verifies replay of:

- the initial Bingo bulk build
- incremental inserts
- non-key updates that invoke Bingo
- DELETE/VACUUM changes
- `REINDEX` relation replacement

It waits for the standby to replay through the primary's target WAL LSN and compares Bingo query results between primary and standby.

The wrapper intentionally removes the harness volumes before this suite so the standby always comes from a fresh base backup of the primary under test.

## Direct Compose use

The wrapper is only convenience. The underlying Compose services can be used directly:

```bash
cd bingo/postgres/tests/docker

docker compose build
docker compose up -d primary
docker compose --profile tools run --rm runner normal
docker compose --profile tools run --rm runner concurrent

docker compose --profile crash run --rm crash-test

docker compose --profile replica up -d primary standby
docker compose --profile replica run --rm replica-test replica
```

## Inspecting the normal primary

```bash
bash bingo/postgres/tests/docker/run.sh shell
```

or from the host:

```bash
psql 'host=127.0.0.1 port=55432 dbname=test user=postgres password=postgres'
```

Follow PostgreSQL logs with:

```bash
bash bingo/postgres/tests/docker/run.sh logs
```

## Test image provenance

The harness deliberately does not download Bingo by version number. Docker receives the local `libbingo-postgres.so`, so the tests exercise the exact binary supplied by the caller. This avoids the earlier ambiguity where an image tag could say one Bingo version while containing a different shared object.

If the shared object changes, run `build` again before testing. Docker will invalidate the `COPY libbingo-postgres.so` layer and rebuild the image with the new artifact.
