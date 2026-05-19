# Bingo Oracle — Local Docker Test Harness

Run the same pytest suite that CI executes (`pytest --db oracle`) entirely in
Docker, with no Oracle SDK required on the host.

Mirrors the `test_bingo_oracle_linux_x86_64` CI job in
`.github/workflows/indigo-ci.yaml` (lines 1089–1182).

## Requirements

- Docker Desktop. On Apple Silicon, enable Rosetta — Oracle XE has no arm64
  image so the whole stack runs as `linux/amd64` under emulation.
- ~10 GB free disk for images and the `bingo-oracle` build volume.

## One-shot run

From the repo root, in two steps:

    # 1. Build the bingo-oracle tarball into dist/
    docker compose -f bingo/oracle/tests/docker/docker-compose.yml run --rm builder

    # 2. Build the self-contained Oracle + bingo image, start it, run the suite
    docker compose -f bingo/oracle/tests/docker/docker-compose.yml up --build \
        --abort-on-container-exit --exit-code-from tests oracle tests

The split is mandatory: `bingo/oracle/Dockerfile` `COPY`s `dist/bingo-oracle-linux-*.tgz`,
so the builder must produce that artifact before the `oracle` image can be built.

What happens:
1. `builder` — compiles `bingo-oracle` in a CentOS 7 buildpack, drops `dist/bingo-oracle*.tgz`.
2. `oracle` — image built from `bingo/oracle/Dockerfile` (`gvenzl/oracle-xe:21-slim` + the bingo tarball + initdb/startdb hooks). On first DB init, `initdb-bingo.sh` runs `bingo-oracle-install.sh` and creates the `test/test` schema user; on every startup, `setup-sqlnet.sh` strips `DISABLE_OOB` so extproc callbacks work. Healthcheck flips green once `test/test` can log in (~2 min on arm64).
3. `tests` — runs `pytest -s --tb=short --db oracle --junit-xml=junit_report.xml` against `oracle:1521` after the healthcheck passes.

JUnit report lands at `bingo/tests/junit_report.xml`.

## Iteration loop

Keep Oracle running between test invocations:

    # Once — build the bingo tarball, then bring up Oracle with the cartridge baked in
    docker compose -f bingo/oracle/tests/docker/docker-compose.yml run --rm builder
    docker compose -f bingo/oracle/tests/docker/docker-compose.yml up -d --build oracle

    # Then iterate — re-run only the tests service, skipping dependencies
    docker compose -f bingo/oracle/tests/docker/docker-compose.yml run --rm --no-deps tests \
        pytest -s --db oracle test_substructure/

After C++ changes, rerun `builder` to refresh the tarball, then rebuild the `oracle`
image so the new artifact is baked in. `down -v` matters here: `initdb-bingo.sh` only
runs on first DB init, so a surviving oradata volume keeps the previous `.so` and SQL
state regardless of the rebuilt image.

    docker compose -f bingo/oracle/tests/docker/docker-compose.yml run --rm builder
    docker compose -f bingo/oracle/tests/docker/docker-compose.yml down -v
    docker compose -f bingo/oracle/tests/docker/docker-compose.yml up --build \
        --abort-on-container-exit --exit-code-from tests oracle tests

## Full reset

    docker compose -f bingo/oracle/tests/docker/docker-compose.yml down -v
    rm -rf dist build

## Interactive shell

    docker compose -f bingo/oracle/tests/docker/docker-compose.yml run --rm --no-deps tests bash

From inside the container, `sqlplus system/password@oracle:1521/XEPDB1` hits
the DB directly; `sqlplus test/test@oracle:1521/XEPDB1` hits the test schema.

## Notes

- The CI source of truth is `.github/workflows/indigo-ci.yaml`. If CI changes,
  mirror those changes here.
- The bingo install and `test/test` user creation live in `initdb-bingo.sh`,
  which gvenzl runs as a `/container-entrypoint-initdb.d/` hook on the **first**
  DB init only. To re-install after destructive changes, use the full-reset
  procedure above — the surviving oradata volume keeps the previous state
  even if you rebuild the `oracle` image.
- First build is slow (~10–20 min on arm64): CentOS 7 buildpack compiles the
  full Indigo core + bingo-oracle, and Oracle Instant Client zips are ~150 MB.
  Subsequent runs reuse cached layers.
