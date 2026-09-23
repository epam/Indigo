---
name: indigo-testing
description: "Decide which Indigo test suites a change must pass, know what CI runs for a pull request and what it silently skips, and sort a red run into the change, the build or the machine before calling it a regression."
when_to_use: "Use before calling a change tested, when a local or CI run is red and the cause is not yet known, when a suite is green in one place and red in another, or when asked where test coverage is thin. To write a test use /indigo-add-tests; for a reference test in api/tests/integration use /indigo-reference-tests."
argument-hint: "[changed area, failing suite or run log]"
allowed-tools: Bash(python api/tests/integration/test.py *) Bash(python3 api/tests/integration/test.py *) Bash(ctest *) Bash(git status *) Bash(git diff *)
---

# Testing an Indigo change

Target: **$ARGUMENTS**

How to build and run each suite is in [testing.md](../../../.memory-bank/testing.md). Writing a test is
`/indigo-add-tests`; a reference test in the integration harness is `/indigo-reference-tests`. This skill
decides what a change has to pass, says what CI will and will not run for it, and sorts a red run before
anyone calls it a regression.

## 1. Which suites the change must pass

Take the areas from `git diff --stat <base>...HEAD` and run every row they hit. The last column is what the
pull request's workflow runs by itself; everything else is yours to run.

| The diff touches | Run locally | CI runs |
| --- | --- | --- |
| `core/indigo-core/` | core GTest; the harness under CPython on Windows **and** Linux | ctest; the harness under CPython and Jython on Linux and Windows, under IronPython on Windows |
| the layout, or anything that writes coordinates | the above, with the layout references checked on both platforms (`/indigo-reference-tests`, §5) | the same; macOS is not run for a pull request |
| `core/render2d/` | `rendering/*` in the harness; `indigo-cpp-unit-tests` | as for the core |
| `api/c/` | `indigo-api-unit-tests`; the harness under CPython | ctest with the `dlopen` smoke tests; the harness under all three interpreters |
| `api/cpp/` | `indigo-cpp-unit-tests` | ctest |
| `api/python/` | `python -m pytest tests` from `api/python/`; the harness under CPython | the `unittest` classes only (§2); static analysis |
| `api/java/` | `mvn test -pl <module>` from `api/java/`; the harness under Jython | the JUnit tests on Linux, JDK 8; the harness under Jython |
| `api/dotnet/` | `dotnet test api/dotnet/tests`; the harness under IronPython | the MSTest tests on Linux, .NET 6; the harness under IronPython on Windows |
| `api/http/` | `pytest` in `api/http/`, on Linux, with the wheel of your build installed — `requirements.txt` pins `epam.indigo` from PyPI | static analysis only (§2) |
| `api/wasm/` | the `indigo-ketcher-package` target, which runs `npm test` | the same target |
| `bingo/postgres/`, `bingo/oracle/` | `pytest --db <engine>` from `bingo/tests/`, the engine in Docker | PostgreSQL 14–18 and Oracle, on Linux |
| `bingo/bingo-elastic/` | `pytest tests` and `mvn test` in its folders, with a live Elasticsearch | both, on Linux |
| `api/tests/integration/` | `/indigo-reference-tests` | the harness under all three interpreters |

## 2. What a green check does and does not cover

Verified against `.github/workflows/indigo-ci.yaml` and the job logs of run 7682 (PR #3912, `97a183d5c`).

- **CI runs for pull requests and `indigo-*` tags only.** The `push` trigger on `master` is commented
  out, so a merge result is not built again.
- **The jobs form a chain.** Static analysis, then the native libraries with ctest, then the wrappers with
  their unit tests, then the harness under CPython, Jython and IronPython, then Bingo and the services. A
  red job leaves every job after it *skipped*, and skipped is not passed — read which jobs ran, not the
  colour of the check.
- **Static analysis stops at its first error.** It runs `clang-format` over the whole tree, then `isort`,
  `black`, `pflake8` and `mypy` folder by folder ([conventions.md](../../../.memory-bank/conventions.md)),
  so each push can uncover only the next failure. Run the whole script in the CI container
  (`/indigo-reference-tests`, §6).
- **The wrappers' unit tests run inside the wrapper builds, on Linux only:** `setup.py test` for Python,
  `mvn install` for Java, `dotnet test` for .NET (`api/*/CMakeLists.txt`).
- **`setup.py test` runs `unittest` classes only.** A test written with pytest fixtures passes locally
  and never runs in CI — `api/python/tests/test_sequence_layout.py` is 20 such tests. The same selection
  locally: `python -m unittest discover -s tests -t .` from `api/python/` (33 tests; `pytest` finds 53).
- **The HTTP service tests run nowhere in CI.** The `pylint`, `mypy` and `pytest` steps sit in the
  `indigo_service_dev` stage of `api/http/Dockerfile`, the image built in CI does not depend on that stage,
  and BuildKit skips it — the job log shows only the final stage.
- **Reduced legs:** Linux ARM64 runs only the `dlopen` smoke tests and `basic/basic.py`; the Windows x86 legs
  run ctest and `basic/basic.py`; IronPython runs on Windows only; macOS runs only for a tag or a manual
  dispatch with `run_mac_tests`; the MinGW harness step is commented out; clang-tidy does not run.

## 3. Prove the run happened before reading its results

The integration harness — save the output, then read it together with the output directory:

```bash
python api/tests/integration/test.py -p <groups> -t 4 -o <out dir> > <log> 2>&1
```

Each test prints one result line, `<group>/<test>.py … [PASSED|FAILED|ERROR|NEW|TODO] <n> sec`. For every
`FAILED` or `ERROR` the evidence is in `<out dir>/<group>/<test>.py.diff` and `<test>.py_0.err`; §4 sorts it.

- **No result lines** means nothing ran, whatever the exit code says. The runner reads its options in
  pairs: `-u` anywhere but last prints `Unexpected options` and exits 0; `-j` names the JUnit report, the
  thread count is `-t`.
- **The runner's exit code is a bit mask:** 1 — a test failed, 2 — a test errored, 4 — a `[NEW]` test had
  no recorded `.out` and compared nothing.
- **`Indigo library path` must point into this checkout**, and the file must be newer than the build
  output it was copied from. The `Indigo version` line is stamped by `git describe` when CMake configures,
  so it proves nothing about freshness.

GTest — run a binary from a scratch directory, because it writes `mcs_test.log`, `test_*.db/` and PNG files
into the working directory, and judge it by its `[  PASSED  ]` and `[  FAILED  ]` lines as well as the exit
code: on Linux all three binaries are built with AddressSanitizer and LeakSanitizer, and a sanitizer report
fails a binary whose assertions all passed.

## 4. Sort each failure: the change, the build, or the machine

Classify only with evidence — a signature from this table, or the same failure in a baseline run.

| Signature | Class | What happened | Next step |
| --- | --- | --- | --- |
| the old behaviour persists after a fix; `Indigo library path` outside the checkout, or its file older than the build | build | the harness imports the first `indigo` on `sys.path` — an installed wheel wins over `api/python/` — and loads the libraries from its `lib/<os>-<arch>/`, which no build refreshes | copy the fresh libraries where the header points, or run without the installed wheel |
| a core change does not reach `rendering/*` | build | `indigo-renderer` links its own static copy of the core | rebuild `indigo-renderer` together with `indigo` |
| `[ERROR] bingo/*`: `Could not find native libraries` | environment | `bingo-nosql` is built with the core but was not copied next to the other libraries | copy it; not a regression |
| `[FAILED] rendering/*`: `similarity is` | environment | raster output depends on the machine's fonts, and a Linux machine can fail these tests on a clean tree | compare with a baseline run on the same machine |
| `FileNotFoundError` or `PermissionError` under the output directory | environment | two runs shared one `-o` — the runner deletes it when it starts | give each run its own `-o` |
| on Linux, `UnsatisfiedLinkError` or `DllNotFoundException` for a library that exists, and `ldd <library>` prints `GLIBC_2.xx not found` | environment | the library was built against a newer glibc than the machine that loads it — a build from a newer distribution run inside the CI image | run where you built |
| `BadImageFormatException (0x8007000B)` from `dotnet test` on Windows | environment | `prebuild.ps1` put empty stubs of `msvcp140.dll` and `vcruntime140*.dll`, missing from `dist/lib/windows-x86_64/`, next to the tests | put the real runtime DLLs there |
| a GTest test with its own `try`/`catch` around C API calls: `thrown in the test body`, in a local MSVC build only | environment | the Ninja generator passes CMake's `/EHsc` and Indigo's `-EHs` together; the `c` survives, and at `/O2` the compiler drops a handler around a call it assumes cannot throw. The Visual Studio generator CI uses keeps the handler — `IndigoSerializeTest.isotopes_basic` and `IndigoApiInchiTest.incorrect_symbols*` are green there | not a regression; new tests use `EXPECT_THROW` (`/indigo-add-tests`) |
| a Linux GTest binary aborted by a sanitizer report inside `libfontconfig` or the renderer's initialisation | environment | a local toolchain difference — CI builds these binaries in a CentOS 7 image | read the stack; blame the change only if a frame is in the code it touched |
| anything else | change | — | read `<out dir>/<group>/<test>.py.diff` and `_0.err` first |

A failure left unattributed belongs to the change until shown otherwise. To show that it predates the
change, run the same test on a build of the merge base in a separate worktree — never regenerate a
reference to turn it green.

## 5. Report

```text
Suites     core GTest <passed>/<total> · C API <passed>/<total> · harness on Windows <passed>/<total>
Platforms  Windows (MSVC, Ninja, Release) · Linux (GCC)
Not run    Jython, IronPython — left to CI · api/http — not touched, and not run by CI
Failures   <group>/<test>.py — environment — <the signature, or the baseline run that fails the same way>
```

A suite that was not run goes under *Not run*, never under *Suites*. An *environment* failure carries its
evidence; a guess is reported as unattributed.

## 6. Where the coverage is thin

Verified on `60b769879` and CI run 7682, in the order they are likely to cost something:

1. **The HTTP service tests never run in CI** (§2) — nothing there catches a regression in `api/http`.
2. **pytest-style Python tests never run in CI** (§2) — the sequence layout's tests are among them.
3. **No fuzzing of the parsers.** MOL, SDF, SMILES and KET read untrusted input, and there is no
   corpus-driven harness for them.
4. **No property-based tests** — no Hypothesis, no jqwik. Round trip is the natural property here:
   whatever loads survives save → load with the same canonical SMILES.
5. **No performance regression tests** for fingerprints, substructure search or the layout; a slowdown is
   noticed by users, not by CI.
6. **Platform gaps for a pull request:** no macOS, no ARM64 beyond `basic/basic.py`, the .NET wrapper
   integration on Windows only, the Java and .NET unit tests on Linux only, no MinGW harness.

## Checklist

- [ ] Every area in the diff mapped to its suites (§1), and each one run or listed as not run
- [ ] Each harness run: result lines present, `Indigo library path` in this checkout, exit code read as a bit mask
- [ ] Coordinates or rendering touched → run on Windows and on Linux, each with its own `-o`
- [ ] Python wrapper tests are `unittest` classes, or they do not run in CI
- [ ] Every failure classified with its evidence; the unattributed ones read, not waved off
- [ ] The report lists what was not run, and why
