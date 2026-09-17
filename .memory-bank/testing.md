# Testing

> **Read when:** you need to run, extend, or debug a suite — native GTest, the cross-language
> integration harness, a wrapper's own tests, WASM, Bingo, or the REST service.
> **Skip when:** you are not running tests in this task.

Verified on `f0cc3c423`. Oracle has its own document because of the Docker harness and its traps —
[modules/bingo-oracle.md](./modules/bingo-oracle.md).

## The thing to understand first

`api/tests/integration/test.py` is **not a Python test suite.** It is the cross-language conformance
harness: CI runs the same file under three interpreters, and each run exercises a different wrapper.

| Interpreter | What it actually tests |
| --- | --- |
| CPython | the Python wrapper (`api/python/`) |
| **Jython** — `java -Xss4m -jar jython.jar api/tests/integration/test.py` | the **Java** wrapper |
| **IronPython** — `dotnet IronPython/net6.0/ipy.dll api/tests/integration/test.py` (and `ipy.exe`, `ipy32.exe` on Windows) | the **.NET** wrapper |

So a change to the C API is verified across three language bindings by one suite, and a test added
there must not use anything CPython-specific — it has to run under Jython 2.7 and IronPython too.
Results are compared against recorded baselines in `api/tests/integration/ref/`.

```bash
pip install dist/epam.indigo-*.whl

python api/tests/integration/test.py -t 1                 # everything
python api/tests/integration/test.py -t 1 -p basic/basic.py  # one file
python api/tests/integration/test.py -t 1 -j junit_report.xml
```

Under CPython the harness *appends* `api/python` to `sys.path`, so an installed `epam.indigo` wins over
the checkout, and the libraries are loaded from that package's `lib/<os>-<arch>/`, which no build
refreshes. The `Indigo library path` line of the header shows which library ran.

## Native unit tests — GTest through CTest

Three GTest binaries plus a set of `dlopen` smoke tests, all registered with CTest:

| Target | Covers | Registered in |
| --- | --- | --- |
| `indigo-core-unit-tests` | core algorithms | `core/indigo-core/tests/CMakeLists.txt#indigo-core-unit-tests` |
| `indigo-api-unit-tests` | the C API | `api/c/tests/unit/CMakeLists.txt#indigo-api-unit-tests` |
| `indigo-cpp-unit-tests` | the C++ wrapper — `basic`, `bingo`, `formats`, `inchi`, `rendering`, `substructure` | `api/cpp/tests/CMakeLists.txt#indigo-cpp-unit-tests` |
| `dlopen-*` | that each shared library loads: indigo, indigo-inchi, indigo-renderer, bingo-nosql | `api/c/tests/dlopen/CMakeLists.txt#add_test(NAME` |

```bash
cmake --build --preset indigo-debug
ctest --verbose                    # from the build directory; CI uses exactly this
ctest --verbose -C Release         # multi-config generators
ctest --verbose -R dlopen          # smoke only — what the ARM and cross-built legs run
```

The binaries can also be run directly (`./bin/indigo-core-unit-tests`), which is the faster loop
when iterating on one test. On Windows, make sure the DLL next to the binary is the one you just
built — a stale copy produces failures that look like logic errors.

## Wrapper-specific tests

Verified against the job logs of CI run 7682 (PR #3912, `97a183d5c`).

| Suite | Location | Run with | In CI |
| --- | --- | --- | --- |
| Python wrapper | `api/python/tests/` — indigo, inchi, renderer, bingo-nosql, sequence layout | `pytest` | the `unittest` classes only, 33 of 53 — `setup.py test` in the `indigo-python` target |
| Java JUnit | `api/java/*/src/test/java/com/epam/indigo/` — `IndigoTests`, `IndigoInchiTests`, `IndigoRendererTests`, `BingoTests` | `mvn test` | on Linux with JDK 8 — `mvn install` in the `indigo-java` target |
| .NET | `api/dotnet/tests/` — `IndigoTest.cs`, `InchiTests.cs` | `dotnet test` | on Linux with .NET 6 — `dotnet test` in the `indigo-dotnet` target |
| **HTTP service** | `api/http/tests/` | `pytest` with `httpx` | **no** |

The wrapper unit tests run while the `build_indigo_wrappers` job builds each wrapper in the
`epmlsop/indigo-tester` container — on Linux, and nowhere else. Two gaps follow from how they are run:

- **`setup.py test` collects `unittest.TestCase` classes only.** `test_sequence_layout.py` is written
  with pytest fixtures, and its 20 tests never run in CI. `python -m unittest discover -s tests -t .`
  from `api/python/` selects exactly what CI runs.
- **The HTTP service tests do not run at all.** `pylint`, `mypy` and `pytest` sit in the
  `indigo_service_dev` stage of `api/http/Dockerfile`; CI builds the final image, which does not depend
  on that stage, and BuildKit skips it — the job log shows the final stage only. Run them locally, on
  Linux, with the wheel of your build installed: `api/http/requirements.txt` pins `epam.indigo` from PyPI.

## WASM

The JavaScript tests for `indigo-ketcher` run as part of packaging: the
`indigo-ketcher-package` target invokes `npm test` (and `npm run test_cjk` in the CJK
configuration) before `npm pack`, so a failing JS test fails the package build. There is no separate
CI step to look for.

```bash
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build . --target indigo-ketcher-package
```

## Bingo

The cartridge suites run one test body against several database engines. Configure connections in
`bingo/tests/db_config.ini` first, and **run from `bingo/tests/`** — `base.SQLAdapter` resolves that
file relative to the working directory.

```bash
pip install -r bingo/tests/requirements.txt
cd bingo/tests

pytest .                    # default: bingo-nosql
pytest --db postgres
pytest --db oracle          # see modules/bingo-oracle.md
pytest --db bingo-elastic
pytest test_exact/test_exact.py --db postgres
```

### The adapter pattern that makes `--db` work

`bingo/tests/dbc/` holds one adapter per engine — `PostgresSQL.py`, `OracleDB.py`, `BingoNoSQL.py`,
`BingoElastic.py` — all deriving from `base.SQLAdapter`. The root `conftest.py` reads `--db` and
injects the matching adapter as the `db` fixture; each `test_*/conftest.py` handles its own data.

Every adapter method (`checkmolecule`, `aam`, `substructure`, `similarity`, …) **returns** a value, a
list, or an `Exception` — it never raises. That is what lets one test body assert parity across
engines whose error reporting has nothing in common. Errors are recognised by prefix: `ORA-`,
`bingo:`, `(oracledb.exceptions.` for Oracle; `bingo:`, `<class '` for Postgres. A method that
raises instead of returning breaks every cross-engine comparison.

### Bringing the engines up

```bash
docker build --tag epmlsop/bingo-postgres:latest \
  -f bingo/postgres/Dockerfile --build-arg BINGO_PG_VERSION=14 .
docker run -d -p 5432:5432 -e "POSTGRES_PASSWORD=password" epmlsop/bingo-postgres:latest

docker run -p 9200:9200 \
  --env "discovery.type=single-node" \
  --env "indices.query.bool.max_clause_count=4096" \
  docker.elastic.co/elasticsearch/elasticsearch:7.17.11
```

### Bingo-Elastic clients

Separate products with their own suites: `pytest tests` in `bingo/bingo-elastic/python/`, and
`mvn clean test` in `bingo/bingo-elastic/java/` — both need a live Elasticsearch.

## indigo-service

```bash
export INDIGO_SERVICE_URL=http://localhost:5000/v2
cd utils/indigo-service/backend/service && cp v2/common/config.py .

waitress-serve --listen="127.0.0.1:5000 [::1]:5000" app:app   # terminal 1
python utils/indigo-service/backend/service/tests/api/indigo_test.py   # terminal 2
```

## Rules that keep results honest

- **Read the exit code.** A suite that was not run is reported as not run, never as passing.
- **2D coordinates are the same on Windows and Linux, not on every platform** — macOS on ARM can
  still differ ([invariants.md](./invariants.md), A10). A reference with coordinates is generated on
  one platform and checked on the other; a per-platform reference is added only for a platform whose
  output genuinely differs.
- **Assert the exception message, not only that something failed** — the text is a public contract.
- **Reference files in `ref/` are regenerated deliberately** with `test.py … -u`. If a baseline
  moved, explain what changed in the behaviour and why the new value is right. How a test in this
  harness is written — the comparison helpers, the fixture and reference layout, the per-platform
  references — is in [.claude/rules/integration-tests.md](../.claude/rules/integration-tests.md).
- **Anything added to the integration harness must run under Jython and IronPython**, not only
  CPython.
