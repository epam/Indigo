# Bingo PostgreSQL 17 A/B recovery matrix

The regression harness can build either the current working tree or a committed
Git ref while always running the current test scripts. This lets the unpatched
1.45 release and the patched 1.45 branch run under the same PostgreSQL 17 image,
configuration, and workload.

Unpatched 1.45 release commit:

```text
ba44a12bb3ae1923ade3395402ab451c3453c9ca
```

## Recovery cases

The `crash` command now consists of three isolated tests. Each gets fresh tmpfs
`PGDATA` and uses `pg_ctl stop -m immediate` to model loss of PostgreSQL shared
buffers during an unclean Kubernetes/node shutdown.

- `recovery-build`: checkpoint seed heap, commit `CREATE INDEX`, stop immediately
  with no post-build checkpoint, restart, compare Bingo search to heap truth.
- `recovery-committed`: checkpoint a healthy Bingo index, commit index-extending
  inserts, stop/restart, then execute the same non-HOT unchanged-Bingo-key UPDATE
  shape that exposed the production corruption. The updated B-tree value is the
  SMILES from the production stack trace. The committed update is itself followed
  by another immediate stop/restart.
- `recovery-inflight`: stop PostgreSQL while INSERT/UPDATE/VACUUM work is active
  and verify rollback visibility, continued writes, search correctness, and safe
  handling/reuse of any unpublished tail allocation.

Run them individually:

```bash
bash bingo/postgres/tests/docker/run.sh recovery-build
bash bingo/postgres/tests/docker/run.sh recovery-committed
bash bingo/postgres/tests/docker/run.sh recovery-inflight
```

Or all three:

```bash
bash bingo/postgres/tests/docker/run.sh crash
```

## Unpatched control

From the patched branch checkout, build the 1.45 release source but retain the
current test harness:

```bash
BASE=ba44a12bb3ae1923ade3395402ab451c3453c9ca

BINGO_SOURCE_REF="$BASE" \
  bash bingo/postgres/tests/docker/run.sh recovery-build

BINGO_SOURCE_REF="$BASE" \
  bash bingo/postgres/tests/docker/run.sh recovery-committed

BINGO_SOURCE_REF="$BASE" \
  bash bingo/postgres/tests/docker/run.sh recovery-inflight

BINGO_SOURCE_REF="$BASE" \
  bash bingo/postgres/tests/docker/run.sh replica
```

The build output and test startup print the selected source commit and the SHA-256
of `libbingo-postgres.so`. A baseline failure can surface as the original
`data len is -8`, an uninitialized/zero page, a search-vs-heap mismatch, or a
standby replay/query failure. Exact failure text depends on which damaged page is
first reached.

If an unpatched recovery case occasionally survives because dirty pages happened
to be written before the stop, repeat that case. The crash instance disables
background-writer LRU flushing and uses a long checkpoint timeout specifically to
make the missing-WAL behavior easier to reproduce without changing WAL/fsync
semantics.

## Patched branch

Return to the default working-tree build by leaving `BINGO_SOURCE_REF` unset:

```bash
unset BINGO_SOURCE_REF

bash bingo/postgres/tests/docker/run.sh recovery-build
bash bingo/postgres/tests/docker/run.sh recovery-committed
bash bingo/postgres/tests/docker/run.sh recovery-inflight
bash bingo/postgres/tests/docker/run.sh replica
```

Then run the complete PostgreSQL-specific regression set:

```bash
bash bingo/postgres/tests/docker/run.sh all
```

## Expected evidence matrix

| Case | Unpatched 1.45 | Patched 1.45 |
| --- | --- | --- |
| normal functional/WAL lifecycle without crash | may pass | must pass |
| concurrent writers/non-HOT updates | may reproduce old race | must pass |
| committed CREATE INDEX + immediate stop | expected to expose missing build WAL | must pass |
| committed incremental writes + immediate stop | expected to expose missing page WAL | must pass |
| production-shaped non-HOT update after restart | may discover prior damage, including `data len is -8` | must pass |
| in-flight immediate-stop recovery | unsafe/may fail | must pass |
| physical standby replay | expected to expose missing custom-page WAL | must pass |
| complete `all` suite | not a baseline requirement | must pass |

The proof does not require every stock-1.45 invocation to fail. The important A/B
result is that one or more recovery/replay cases reproducibly expose the no-WAL
defect in the unpatched binary while all corresponding cases and the complete
suite pass with the patched binary.
