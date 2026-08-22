# Bingo PostgreSQL WAL and corruption regression tests

These tests cover the PostgreSQL Bingo storage fixes on the `fix/postgres-bingo-index-corruption-1.45` branch. Build and install the patched `libbingo-postgres.so` and the normal Bingo SQL objects before running them.

The tests deliberately use SQL-level Bingo indexes rather than unit-test mocks. WAL correctness depends on PostgreSQL buffer management, crash recovery, and physical replay, so meaningful coverage requires a real PostgreSQL server.

## 1. Concurrent mutation regression

Runs concurrent direct inserts and concurrent updates to a different indexed column. The latter reproduces the PostgreSQL behavior where a non-HOT update creates a new heap TID and therefore invokes `bingo_insert()` even though the molecule key is unchanged.

```bash
PSQL=/usr/local/pgsql/bin/psql \
WORKERS=8 \
bash bingo/postgres/tests/local/postgres_concurrent_update_test.sh test
```

Increase `WORKERS` and `ROWS` for stress runs:

```bash
WORKERS=16 ROWS=20000 \
bash bingo/postgres/tests/local/postgres_concurrent_update_test.sh test
```

## 2. Build, incremental mutation, VACUUM, and REINDEX integrity

This test does not crash the server. It covers:

- `CREATE INDEX` bulk-build WAL
- transaction rollback
- normal incremental insertion
- non-key indexed-column updates
- `DELETE` + `VACUUM` Bingo removals
- `REINDEX`
- post-build and post-REINDEX Bingo searches
- physical tail-page validation with `pageinspect`
- WAL-volume reporting for build and REINDEX

Run it against a test database as a user that can create `pageinspect`:

```bash
PSQL=/usr/local/pgsql/bin/psql \
bash bingo/postgres/tests/local/postgres_wal_build_test.sh test
```

For a larger build:

```bash
ROWS=50000 bash bingo/postgres/tests/local/postgres_wal_build_test.sh test
```

The WAL byte counts are informational. The script intentionally does not enforce a fixed WAL-volume threshold because PostgreSQL settings such as `wal_compression`, page size, and checkpoint timing affect the result.

## 3. Immediate-stop crash recovery

**This test intentionally kills PostgreSQL with `pg_ctl stop -m immediate`. Run it only against a disposable PostgreSQL cluster.**

The script refuses to start unless `BINGO_WAL_TEST_ALLOW_CRASH=1` is set and `PGDATA` is explicitly supplied.

It covers:

- crash during direct Bingo insertion/page growth
- recovery of uncommitted index changes
- successful writes and searches after recovery
- crash during an unrelated indexed-column update that invokes Bingo
- VACUUM/Bingo removal recovery when the VACUUM remains active long enough to hit the crash point
- reuse of unpublished tail pages left by an interrupted extension
- post-recovery tail-page validation

Example:

```bash
BINGO_WAL_TEST_ALLOW_CRASH=1 \
PGDATA=/tmp/bingo-pg17-crash-test \
PSQL=/usr/local/pgsql/bin/psql \
PG_CTL=/usr/local/pgsql/bin/pg_ctl \
bash bingo/postgres/tests/local/postgres_wal_recovery_test.sh test
```

The disposable server must already be running before the script starts. `pg_ctl` must manage the same `PGDATA` used by the `psql` connection.

If the insert or update finishes before the configured crash point, increase `ROWS` / `CRASH_ROWS` or reduce `CRASH_DELAY`:

```bash
ROWS=100000 \
CRASH_ROWS=300000 \
CRASH_DELAY=0.10 \
BINGO_WAL_TEST_ALLOW_CRASH=1 \
PGDATA=/tmp/bingo-pg17-crash-test \
bash bingo/postgres/tests/local/postgres_wal_recovery_test.sh test
```

The default crash test performs two guaranteed immediate-stop cycles: one during INSERT and one during the non-key UPDATE. It also tries to crash during VACUUM if VACUUM is still running at the configured delay; a faster completed VACUUM is accepted and followed by structural validation.

## 4. Physical standby replay

This test requires an existing PostgreSQL physical streaming-replication pair. The patched Bingo shared library must be installed on both primary and standby. The standby must permit hot-standby reads.

It covers:

- replay of a freshly built Bingo index
- replay of incremental Bingo inserts
- replay of non-key updates
- replay of Bingo VACUUM removals
- representative Bingo queries on the standby
- replay of a `REINDEX` relation replacement

Example:

```bash
PRIMARY_DSN='host=primary.example dbname=test user=postgres' \
STANDBY_DSN='host=standby.example dbname=test user=postgres' \
WAIT_SECONDS=120 \
bash bingo/postgres/tests/local/postgres_wal_replica_test.sh
```

The test waits until `pg_last_wal_replay_lsn()` on the standby reaches the primary's flush LSN before comparing results.

## Recommended verification sequence

After building the patched extension, run:

```bash
bash bingo/postgres/tests/local/postgres_wal_build_test.sh test
bash bingo/postgres/tests/local/postgres_concurrent_update_test.sh test
```

Then run the immediate-crash test on a disposable cluster. If a physical standby is available, run the replica test last.

For higher-confidence stress validation, repeat the concurrent and crash tests with larger row/worker counts rather than changing the production code or adding special production failpoints.

## What the tests should catch

The original corruption signature was a Bingo tail page with an all-zero PostgreSQL page header followed by an error such as:

```text
bingo buffer: internal error: corrupted block ... data len is -8
```

The tests fail if the current Bingo tail contains zero/uninitialized pages after build/recovery, if Bingo searches fail or change unexpectedly across REINDEX/replay, if uncommitted crash workload becomes heap-visible, or if the index cannot accept new writes after recovery.

## Deliberate scope limits

These tests do not change the Bingo SQL API or on-disk format. They do not attempt to repair arbitrary corruption inside the published portion of an existing index. An index that was already corrupted before installing the patched library still needs to be rebuilt with `REINDEX`.
