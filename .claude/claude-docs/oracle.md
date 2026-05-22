# Oracle

Everything specific to running Bingo on Oracle: the local Docker harness, running tests from the host venv, and the load-bearing gotchas in the install scripts and C++ context loader.

## Local Docker harness — `bingo/oracle/tests/docker/`

Mirrors the `test_bingo_oracle_linux_x86_64` CI job in `.github/workflows/indigo-ci.yaml`. Both build the same self-contained image from `bingo/oracle/Dockerfile` (`FROM gvenzl/oracle-xe:21-slim` + the bingo artifact + initdb/startdb hooks), so local results predict CI results. All services run as `linux/amd64` (Oracle XE has no arm64 image — enable Rosetta on Apple Silicon).

Three compose services:

- `builder` — one-shot CentOS 7 buildpack that compiles `bingo-oracle*.tgz` into `dist/`
- `oracle` — built from `bingo/oracle/Dockerfile`; bakes in `extproc.ora`, `setup-sqlnet.sh` (startdb hook that strips `DISABLE_OOB`), and `initdb-bingo.sh` (initdb hook that runs `bingo-oracle-install.sh` and creates the `test/test` schema user with all required grants on first DB init)
- `tests` — runs `pytest --db oracle` from the bridge network as `oracle:1521` (no Docker port proxy in the path)

### Run modes

Compose builds images upfront, so the `.tgz` must exist before the `oracle` image is built. Two-step invocation:

```bash
# Step 1: produce dist/bingo-oracle*.tgz
docker compose -f bingo/oracle/tests/docker/docker-compose.yml run --rm builder

# Step 2: build the oracle image (which COPYs the .tgz in) and run the suite
docker compose -f bingo/oracle/tests/docker/docker-compose.yml up --build \
    --abort-on-container-exit --exit-code-from tests oracle tests

# Iteration loop (keep DB + extension between runs)
docker compose -f bingo/oracle/tests/docker/docker-compose.yml up -d --build oracle
docker compose -f bingo/oracle/tests/docker/docker-compose.yml run --rm --no-deps tests \
    pytest -s --db oracle test_substructure/

# Full reset
docker compose -f bingo/oracle/tests/docker/docker-compose.yml down -v && rm -rf dist build
```

### Re-running after source changes

After C++ changes, rerun `builder` to refresh `dist/bingo-oracle*.tgz`, then rebuild the `oracle` image so the new artifact is baked in:

```bash
docker compose -f bingo/oracle/tests/docker/docker-compose.yml run --rm builder
docker compose -f bingo/oracle/tests/docker/docker-compose.yml build oracle
docker compose -f bingo/oracle/tests/docker/docker-compose.yml down -v   # discard old initdb state
docker compose -f bingo/oracle/tests/docker/docker-compose.yml up --build \
    --abort-on-container-exit --exit-code-from tests oracle tests
```

The `down -v` is what matters: `initdb-bingo.sh` only runs on **first** DB init, so a surviving oradata volume keeps the previous `.so` and SQL state regardless of the rebuilt image.

## Running Bingo tests from host `.venv` against Dockerized Oracle

`python-oracledb` runs in thin mode by default — no Oracle Instant Client SDK required on the host. SQLAlchemy version is pinned: tests use the 1.x API (`connection.execute(query, params)`) which was removed in 2.0.

```bash
.venv/bin/pip install \
  "SQLAlchemy==1.3.22" "oracledb" "epam.indigo" \
  "xmldiff==2.4" "elasticsearch==7.10.1" "setuptools<71"
```

`setuptools<71` matters: `xmldiff 2.4` imports `pkg_resources` at top level; setuptools 71+ removed it.

`bingo/tests/db_config.ini` is read as a relative path from `base.SQLAdapter`, so tests must be run from `bingo/tests/`:

```bash
cd bingo/tests
../../.venv/bin/pytest test_aam/ -v --db oracle
```

## `cx_Oracle` compatibility shim

`bingo/tests/dbc/OracleDB.py` patches `oracledb` as `cx_Oracle` at import time (SQLAlchemy 1.3.22 predates the native `oracledb` dialect). The engine still uses the `oracle+cx_oracle://` connection string. CLOB binds are explicit (`sa.bindparam("query_entity", type_=CLOB)`) — without this, oracledb sends VARCHAR2 and large molecules silently truncate.

## `_init_bingo_context` connect-event hook

`OracleDB.py` registers a SQLAlchemy `connect` event that calls `bingo.ConfigSetInt(0, 'ct_format_save_date', 0)` on every fresh DB connection (deterministic test output without dates). The hook commits **immediately** afterward — `ConfigSetInt` does `DELETE+INSERT` on `BINGO.CONFIG_INT`, and without an immediate commit the row lock is held for the connection's lifetime, blocking any other test session that calls `ConfigSetInt`.

Symptom of forgetting the commit: pytest hangs at the first test with no output.

## `DPY-6005: cannot connect to database` — instance crashed, listener still up

The Oracle XE container can stay healthy (TCP/1521 reachable) while the database instance itself is down — typically after the host sleeps or after a memory-heavy import (Oracle XE has a hard 2 GB SGA cap; `test_bigtable` imports 100k molecules and generates a lot of redo). The listener answers, but the only registered service is `PLSExtProc`; `XEPDB1` is gone, and python-oracledb surfaces it as `DPY-6005`.

Diagnose:

```bash
docker exec bingo-oracle-tests-oracle-1 lsnrctl status | grep '^Service'
# If you don't see "xepdb1" / "XE" listed, the DB is down.

docker exec bingo-oracle-tests-oracle-1 sqlplus -s / as sysdba <<<'SELECT open_mode FROM v$pdbs; EXIT;'
# ORA-01034: ORACLE not available  →  instance is gone
```

Recover (data persists in the named volume):

```bash
docker exec bingo-oracle-tests-oracle-1 bash -lc \
  'printf "STARTUP\nALTER PLUGGABLE DATABASE XEPDB1 OPEN;\nEXIT;\n" | sqlplus -s / as sysdba'
```

`STARTUP` and the `ALTER` must be on separate lines — SQL*Plus parses `STARTUP; ALTER …` as one statement and fails with `SP2-0714: invalid combination of STARTUP options`. On 21c XE, `STARTUP` alone usually auto-opens the PDB; the `ALTER PLUGGABLE DATABASE … OPEN` then returns the harmless `ORA-65019: pluggable database XEPDB1 already open`. PMON re-registers `XEPDB1` with the listener within a few seconds and `pytest --db oracle` works again.

`initdb-bingo.sh` only runs on first DB init — the bingo install does not re-execute against a zombie PDB. If `STARTUP` doesn't recover the instance, the cleanest path is `docker compose down -v` and `up --build` from a fresh image.

## Extproc / install gotchas

- **`extproc.ora` lives at `$ORACLE_BASE_HOME/hs/admin/`, NOT `$ORACLE_HOME/hs/admin/`.** In the Oracle 21c XE official image: `ORACLE_BASE_HOME=/opt/oracle/homes/OraDBHome21cXE` vs `ORACLE_HOME=/opt/oracle/product/21c/dbhomeXE`. Mounting at `$ORACLE_HOME` silently loses the whitelist and any external-procedure call raises `ORA-28595: Extproc agent: Invalid DLL Path` — even with `EXTPROC_DLLS=ANY`, because extproc finds no config file at all.
- **Test user needs EXECUTE on every BINGO standalone procedure**, not just the three packages (`MangoPackage`, `RingoPackage`, `BingoPackage`). `bingo/oracle/tests/docker/initdb-bingo.sh` (run by gvenzl as a `/container-entrypoint-initdb.d` hook) loops over `dba_objects` to grant on all valid functions/procedures/types/indextypes. Without this, `ConfigSetInt` itself fails with `PLS-00201: identifier 'BINGO.CONFIGSETINT' must be declared`.
- **Test user also needs `EXECUTE ON sys.dbms_crypto`** (granted as SYSDBA at the end of `initdb-bingo.sh`). Used by the test adapter's `compactmolecule`/`fingerprint` paths to hash output for parity with Postgres `digest()`.
- **Test user needs `CREATE SEQUENCE`** in addition to `CREATE TABLE`. `OracleDB.create_data_tables` (`bingo/tests/dbc/OracleDB.py`) declares `sa.Column("id", ..., sa.Sequence(...))`, so SQLAlchemy's Oracle dialect emits `CREATE SEQUENCE` as a separate DDL — `CREATE TABLE` privilege alone is not enough. Missing this grant manifests as `ORA-01031: insufficient privileges` at fixture setup, affecting only the table-based modules (`test_substructure`, `test_bigtable`, `test_markush`, `test_resonance`, `test_sgroups`, `test_tautomers`, `test_pseudoatoms`, `test_similarity`).
- **`gvenzl/oracle-xe:21-slim` ships `DISABLE_OOB=ON` in `sqlnet.ora`** ([gvenzl issue #43](https://github.com/gvenzl/oci-oracle-xe/issues/43)) as a Docker workaround for unreliable TCP urgent data. That setting breaks Oracle's Heterogeneous Services callback path (`horcstt_SwitchToTtc`), used by every BLOB-returning bingo function (`Fingerprint`, `CompactMolecule`, `RFingerprint`, ...). Symptom: `ORA-28579: network error during callback from external procedure agent` at the first `test_fingerprint` / `test_compactmolecule` invocation, while non-callback functions (`getversion`, `CheckMolecule`) pass. Both local and CI override via `bingo/oracle/tests/docker/setup-sqlnet.sh`, baked into the image by `bingo/oracle/Dockerfile` as a `/container-entrypoint-startdb.d/` hook; the script `realpath`s the symlink (`$ORACLE_BASE_HOME/network/admin/sqlnet.ora` → `/opt/oracle/oradata/dbconfig/XE/sqlnet.ora`) and rewrites the target. CI additionally runs the container with `docker run --network host` so client connections from the runner bypass Docker's userland port proxy — that proxy drops the very TCP urgent data the OOB removal needs to flow, surfacing as `ORA-12637: Packet receive failed` from any runner-side `sqlplus`/`cx_Oracle`. **Don't try to bind-mount `sqlnet.ora` directly** — gvenzl's first-init does `mv $ORACLE_HOME/network/admin/sqlnet.ora $ORACLE_BASE/oradata/dbconfig/XE/sqlnet.ora`, which fails with `EBUSY` against a bind mount and crashes the container. Diagnose by inspecting `/opt/oracle/diag/rdbms/xe/XE/trace/XE_ora_*.trc` — the failing trace contains `SendExecCallout: horcstt_SwitchToTtc; NCR code 28579`.

## `BingoOracleContext::_loadConfigParameters` is hand-coded, not table-driven

Postgres's loader iterates every row of `bingo_config` and bingo-sets each one. Oracle (`bingo/oracle/src/oracle/bingo_oracle_context.cpp`) only loads a hand-coded list of params — adding a new tunable requires both an `INSERT` in `bingo/oracle/sql/bingo/bingo_config.sql` AND a `configGetIntDef` line in `_loadConfigParameters`. Forgetting the latter means the value is silently ignored on Oracle even if the row exists.

**Latent bug**: `configGetIntDef` has dead code (`return false;` precedes `value = default_value;`) — currently masked because `int val;` is reused across calls in `_loadConfigParameters`, leaving the previous successful value lying around as a de-facto default. Fix by reordering the two statements if it ever bites.

## C++ cartridge: `mango_*.cpp` and `ringo_*.cpp` are parallel paths

`bingo/oracle/src/oracle/` has two parallel implementations: `mango_*` for molecules, `ringo_*` for reactions. Bugs often live in only one side because most ringo functions were originally cloned from their mango counterparts. Two recurring traps:

- **Fingerprint byte length differs between molecules and reactions.** Molecule fingerprints use `fp_parameters.fingerprintSize()`; reactions use `fp_parameters.fingerprintSizeExtOrdSim() * 2` (canonical references: `bingo/bingo-core-c/src/ringo_core_c.cpp`, `bingo/bingo-core/src/core/ringo_index.cpp`, `bingo/bingo-core/src/core/ringo_substructure.cpp`). Copying the molecule expression into a ringo function returns the BLOB successfully but truncated — downstream MD5 comparisons and index lookups silently diverge from Postgres.

- **Wrap function bodies in `ORA_SAFEBLOCK_BEGIN("name") / ORA_SAFEBLOCK_END`, not bare `ORABLOCK_BEGIN / END`.** The SAFE variant (defined in `bingo_oracle.h`) catches Bingo `Exception` and rethrows as `OracleError("Error: %s", e.message())`, matching the Postgres `CORE_CATCH_REJECT_WARNING(...) / PG_RETURN_NULL()` pattern. Without it, an upstream throw (e.g. `"unknown molecule fingerprint type: sub-res"` — actually thrown from the *reaction* builder at `core/indigo-core/reaction/src/reaction_fingerprint.cpp` despite the misleading "molecule" wording) propagates as a fatal Oracle error where Postgres would have returned NULL.

## Related

- [claude-docs/testing.md](testing.md) — the broader test-suite landscape
- [claude-docs/architecture.md](architecture.md) — the test adapter pattern shared across DB engines
- [claude-docs/build.md](build.md) — building Bingo for Oracle generally
