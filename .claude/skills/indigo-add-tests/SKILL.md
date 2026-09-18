---
name: indigo-add-tests
description: "Add tests to an Indigo module - core GTest, C API, C++ wrapper, Python, Java, .NET - from templates written in each suite's own style, with the failure contract asserted by type and message and proof that the test catches the regression."
when_to_use: "Use when a change to Indigo code needs tests, when a bug fix needs the test that fails without it, or when an error path or an option value has no test. For a reference test in api/tests/integration use /indigo-reference-tests instead."
argument-hint: "[module, class or function to cover]"
---

# Adding tests to Indigo

Target: **$ARGUMENTS**

How to run each suite is in [testing.md](../../../.memory-bank/testing.md) — read it rather than guessing an
invocation. This skill is about *what* to write, *where* it goes, and how to show that it catches the
regression it was written for.

## 1. Find the gap, not the number

Coverage percentage is not the goal; the goal is that the next regression fails a test. Look for:

- error paths that no test provokes — every `throw` the change can reach
- the boundary cases of the data, not of the loop: empty structure, single atom, a query molecule where
  a concrete one is assumed, an aromatic form where the code expects a Kekulé one
- behaviour that differs by option — a function reading a loader or valence option needs a test per
  option value, not one test with the default

## 2. Pick the suite, then its template

| Code under test | New test goes to | Template | What the suite already provides |
| --- | --- | --- | --- |
| core algorithm, `core/indigo-core/` | `core/indigo-core/tests/tests/<subject>.cpp` | [core-gtest](templates/core-gtest.cpp.template) | `IndigoCoreTest`: `loadMolecule`, `loadQueryMolecule`, `smiles`, `dataPath`; `METHANE`, `BENZENE`, `CAFFEINE` |
| C API, `api/c/` | `api/c/tests/unit/tests/<subject>.cpp` | [c-api-gtest](templates/c-api-gtest.cpp.template) | `IndigoApiTest`: a session per test, an error handler that throws, `dataPath` |
| C++ wrapper, `api/cpp/` | `api/cpp/tests/<area>/<subject>.cpp` | [cpp-wrapper-gtest](templates/cpp-wrapper-gtest.cpp.template) | `IndigoSession::create()`, `indigo_cpp::dataPath` |
| Python wrapper | `api/python/tests/test_<subject>.py` | [python-unittest](templates/python-unittest.py.template) | `TestIndigoBase` — `unittest`, not pytest fixtures; `self.indigo` |
| Java wrapper | `api/java/<module>/src/test/java/com/epam/indigo/<Subject>Tests.java` | [java-junit5](templates/java-junit5.java.template) | JUnit 5; a new `Indigo` in each test |
| .NET wrapper | `api/dotnet/tests/<Subject>Test.cs` | [dotnet-mstest](templates/dotnet-mstest.cs.template) | MSTest; `using var indigo` |
| anything the three wrappers share | `api/tests/integration/tests/<group>/` | `/indigo-reference-tests` | reference files, run under CPython, Jython and IronPython |

Nothing needs registering. The three GTest binaries collect their sources with
`file(GLOB_RECURSE … CONFIGURE_DEPENDS)`, so a new `.cpp` is built on the next build; unittest discovery,
Maven surefire (`*Tests`) and the SDK-style .NET project pick new files up the same way. Test data comes
through `dataPath("molecules/…")`, never a path relative to the working directory.

## 3. Write it

Required cases for anything with a public surface:

1. **Valid input** → the expected result, asserted on the value, not on "no exception".
2. **Invalid input** → the exception type **and its message**. In Indigo the message text is a public
   contract; a test that only checks "something was raised" does not protect it.
3. **Empty input** → a defined outcome, whatever it is. Undefined is the bug.
4. **Round trip** — load → modify → save → load → compare, for anything touching serialisation.
5. **Parametrised** — at least three inputs where a single one would prove nothing.

How each suite asserts the failure contract — the templates already do it this way:

| Suite | Type | Message |
| --- | --- | --- |
| core | `catch (Exception& e)` after `FAIL()` | `EXPECT_STREQ(message, e.message())` |
| C API | `EXPECT_THROW(call, Exception)` | handler removed: `indigoSetErrorHandler(nullptr, nullptr)`, `EXPECT_EQ(-1, call)`, `EXPECT_STREQ(message, indigoGetLastError())` |
| C++ wrapper | `EXPECT_THROW` around a `catch (const IndigoException& e)` that rethrows | `ASSERT_STREQ(message, e.what())` inside that catch |
| Python | `with self.assertRaises(IndigoException) as raised:` | `str(raised.exception)` |
| Java | `assertThrows(IndigoException.class, …)` | `e.getMessage()` |
| .NET | `Assert.ThrowsException<IndigoException>(…)` | `e.Message` |

## 4. Prove it catches the regression

A test that has never failed has not been shown to test anything.

1. Run the new test and see it pass — only it, with the filter for its suite:

   | Suite | Run one test |
   | --- | --- |
   | GTest | `bin/<binary> --gtest_filter='<Suite>Test.*' --gtest_repeat=3` — from a scratch directory |
   | Python | `python -m unittest tests.test_<subject>` from `api/python/` |
   | Java | `mvn test -pl <module> -Dtest=<Subject>Tests` from `api/java/` — the module packs the native libraries from `dist/lib/<os>-<arch>/` |
   | .NET | `dotnet test api/dotnet/tests --filter FullyQualifiedName~<Subject>Test` — the native libraries come from `dist/lib/<os>-<arch>/` too |

2. Make it fail on purpose: undo the fix it was written for, or break the expected value. It must fail,
   and its output must say what went wrong. Restore, and run it again.
3. Run it three times (`--gtest_repeat=3` above); a flake that shows once shows in CI.
4. Run a GTest test on Linux as well as on the platform you wrote it on (§5).

## 5. What CI does with it, and what it does not

- **GTest on Linux runs under AddressSanitizer and LeakSanitizer** — the test CMakeLists add
  `-fsanitize=address,leak` on Linux. A leak or a mismatched delete in the new test aborts the binary
  there even when every assertion passed.
- **A try/catch written around C API calls in the test body does not catch in a local MSVC build made
  with the Ninja generator.** The generator passes CMake's `/EHsc` and Indigo's `-EHs` together, the `c`
  survives, and at `/O2` the compiler drops a handler around calls it assumes cannot throw — the exception
  escapes to gtest as "thrown in the test body". CI builds Windows with the Visual Studio generator, which
  keeps the handler, so such a test is green in CI and red locally; `IndigoSerializeTest.isotopes_basic` is
  one. The C API template checks the type with `EXPECT_THROW` and the message the C way, which holds in both
  builds.
- **The wrappers' unit tests run in CI on Linux only**, inside the wrapper builds: `setup.py test`,
  `mvn install`, `dotnet test`. Run them locally on your platform before the push.
- **A Python test must be a `unittest` class.** `setup.py test` does not collect pytest-style tests, so a
  test written with pytest fixtures passes locally and never runs in CI.
- **CI compiles the Java tests with JDK 8.** The pom sets source 1.8 without `release`, so a Java 9+ API
  such as `List.of` compiles under a newer local JDK and fails in CI.
- **`dotnet test` on Windows needs `msvcp140.dll`, `vcruntime140.dll` and `vcruntime140_1.dll` in
  `dist/lib/windows-x86_64/`.** Without them `prebuild.ps1` puts empty stubs next to the tests, and every
  test fails with `BadImageFormatException`.

## Traps

- Never assert on exact 2D coordinates by hand: they match on Windows and Linux but not on every platform
  ([invariants.md](../../../.memory-bank/invariants.md), A10). Keep them in a reference file instead.
- Bingo tests run from `bingo/tests/`, and its adapters **return** exceptions instead of raising them
  (B3, B4). A test that raises breaks the cross-database parity assertions.
- The GTest binaries write `mcs_test.log` into the working directory — run them from a scratch directory
  so it does not end up in a commit.
- New reference files for the integration suite are generated deliberately, never regenerated to make a
  red suite green — `/indigo-reference-tests`.

## Checklist

- [ ] The test is in the suite that owns the code, in the file the table names, from its template
- [ ] Valid, invalid (type **and** message), empty, round trip where serialisation is touched
- [ ] It failed once on purpose, with output that says what went wrong
- [ ] Green three times in a row; a GTest test also green on Linux, under the sanitizers
- [ ] A wrapper test ran locally: a Python one as a `unittest` class, a Java one within the Java 8 API
- [ ] No hand-written coordinates, no stray `mcs_test.log`
