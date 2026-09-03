# Conventions

> **Read when:** you need to know what the style gate checks, or which rule file governs the code
> you are about to touch.
> **Skip when:** you are already working inside a file — the matching `.claude/rules/` file loads by
> itself and carries the detail.

Verified on `f0cc3c423`.

This file is the cross-language index. Per-language detail lives in `.claude/rules/`, which loads
automatically when a matching file is read; it is not repeated here, because two copies of a
convention diverge and then neither can be trusted.

| Area | Rule file | What the gate actually enforces |
| --- | --- | --- |
| C++ | [../.claude/rules/cpp.md](../.claude/rules/cpp.md) | `clang-format -Werror --dry-run` over every `.h/.hpp/.c/.cpp` outside `third_party/` and build directories |
| Python | [../.claude/rules/python.md](../.claude/rules/python.md) | `isort --check`, `black --check`, `pflake8`, then `mypy` |
| Java | [../.claude/rules/java.md](../.claude/rules/java.md) | nothing automated — review only |
| CMake | [../.claude/rules/cmake.md](../.claude/rules/cmake.md) | nothing automated — review only |
| CI, Docker | [../.claude/rules/ci-and-docker.md](../.claude/rules/ci-and-docker.md) | nothing automated — review only |
| .NET, R, WASM | — | no rule file and no gate |

The gate is `.ci/static_analysis_check.sh`, run as the `static_analysis` job. It fails the build, so
a formatting mistake blocks a merge as surely as a compile error.

## C++

Formatting is fixed by `.clang-format`, and the settings worth knowing before you wonder why the
formatter moved something:

- Microsoft base style, **column limit 160** — long lines are the house style here, not an accident
- `PointerAlignment: Left` (`char* p`), `NamespaceIndentation: All`, `AccessModifierOffset: -4`
- `FixNamespaceComments: false`, `AlwaysBreakTemplateDeclarations: Yes`

Run `clang-format -i` on what you touched before pushing; the CI job checks the whole tree and
reports every file, so one unformatted line drowns the output.

**clang-tidy does not run.** `.clang-tidy` exists with a broad check list, and `USE_CLANG_TIDY` is a
CMake option, but the C++ section of `.ci/static_analysis_check.sh` is commented out behind a
`# TODO`, and `WarningsAsErrors` is empty. Enabling it is a project decision with a large one-off
cost; until then, do not assume a clean build means clang-tidy-clean.

## Python

- `black` and `isort` with **line length 79**; `pflake8` (flake8 driven from `pyproject.toml`); then
  `mypy`.
- Style and lint run in `api/http`, `api/python`, `bingo/bingo-elastic/python`,
  `api/tests/integration` and `utils/indigo-service/backend/service`.
- **`mypy` runs in the same set except `api/tests/integration`** — the integration harness is exempt
  from type checking, not from formatting. It has to run under Jython and IronPython
  ([testing.md](./testing.md)), which is also why it cannot use modern CPython-only syntax.

## Java, .NET, R, WASM

No automated style gate. Consistency is a review matter, and the rule that carries the most weight
is the one in [../.claude/rules/java.md](../.claude/rules/java.md): a new wrapper method follows the
shape of the existing ones. A wrapper that is half in one style is harder to use than one that is
uniformly imperfect.

## Everywhere

- **Error message text is a public contract** ([invariants.md](./invariants.md), A2). Wording is not
  a style choice.
- **Comments are paid for**: an invariant a type cannot express, a reference to an external source
  of truth, a trap a test confirms, a short file header. Rationale and rejected alternatives belong
  in the commit message.
- **Named constants, not literals**, for anything with a domain meaning — tests included.
- Commit subject and branch naming are in [../CLAUDE.md](../CLAUDE.md).
