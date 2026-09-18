---
name: indigo-reference-tests
description: "Add or refresh a reference-driven test in Indigo's integration harness (api/tests/integration): choose the comparison, place fixtures, generate references with -u, clean up what the run left, and verify on the second platform and under Jython."
when_to_use: "Use when a change needs a new test in api/tests/integration, when references there must be regenerated after a deliberate behaviour change, or when a reference test passes on one platform and fails on the other."
argument-hint: "[format, feature or test file to cover]"
paths: "api/tests/integration/**"
allowed-tools: Bash(python api/tests/integration/test.py *) Bash(python3 api/tests/integration/test.py *) Bash(git status *) Bash(git diff *) Bash(git ls-files *)
---

# Reference tests in the integration harness

Target: **$ARGUMENTS**

The conventions are in [the harness rule](../../rules/integration-tests.md), which loads by itself when a
file under `api/tests/integration/` is touched; how to run every suite is in
[testing.md](../../../.memory-bank/testing.md). This skill is the order of operations — the part that
costs a review round when it is improvised — and the traps that make a run look green when it did not
happen.

## 1. Decide what the reference holds

A reference file is the assertion. Choose what to save so that the diff a reviewer reads is the change
itself:

| What is tested | Save | Compare with |
| --- | --- | --- |
| a format, a round trip | the saved text — `mol.json()`, `mol.molfile()`, `mol.smiles()` | `compare_diff(ref_path, name, data)` |
| a layout, exactly | KET after `layout()`, three decimals | `compare_diff` |
| a layout, within a tolerance | a molfile | `moleculeLayoutDiff(indigo, mol, "name.mol")` — atom by atom, `delta=0.01`, writes the file under `-u` |
| an API surface, counts, errors | `print()`; errors through `getIndigoExceptionText(e)` | the recorded `.out` |
| a raster render | the image | `checkImageSimilarity`, never a byte diff |

Do not compute the property under test inside the test. If the answer needs geometry, save the structure
and let the reference carry the geometry.

## 2. Place the files, and know where the harness looks

Fixtures join the group's shared `tests/<group>/molecules/`; references go to `tests/<group>/ref/`. A
private fixture folder per test is what the reviewer will ask you to undo.

The harness picks the first file that exists — so a stale platform file silently wins over a correct
shared one:

| Reference | Searched in order |
| --- | --- |
| recorded stdout | `ref/<group>/jython/`, `ref/<group>/iron/`, `ref/<group>/<platform>/`, `ref/<group>/` — `test.py#determine_path_based_on_platform`, and only if the shared `ref/<group>/<subject>.py.out` exists: a platform `.out` on its own is never read |
| data files | `tests/<group>/ref/<platform>/`, `tests/<group>/ref/` — `compare_diff`, `getRefFilepath` |

## 3. Write the test

Start from [templates/reference_test.py.template](templates/reference_test.py.template): a sorted list of
fixtures, one operation, one comparison per file. Name the file for the subject — no `golden`, `new` or
`test_`.

## 4. Generate, then clean up after the run

```bash
python api/tests/integration/test.py -p "<group>/<subject>.py" -t 1 -o <out dir> -u   # -u LAST
git status --porcelain --untracked-files=all -- api/tests/integration                  # what the run left
python api/tests/integration/test.py -p "<group>/<subject>.py" -t 1 -o <out dir>      # green with no -u
```

- **`-u` goes last.** The runner reads options in pairs and `-u` has no value: anywhere else it shifts the
  pairs, the runner prints `Unexpected options`, runs nothing, regenerates nothing — and exits 0. Confirm
  that the result line for the test is in the output.
- **Every changed reference needs a reason, and nothing else stays.** `-u` rewrites every reference the run
  touches, not only the ones you meant to move; an output a test writes into the source tree — such as
  `tests/cano/bugs/` — is deleted, never committed.
- **No CRLF reaches the commit.** `compare_diff` writes in text mode, so on Windows `-u` leaves CRLF. With
  `core.autocrlf` set to `input` or `true` git stores LF on commit — the file then shows as modified while
  `git diff` is empty. With `false` it would store CRLF. After `git add`, check the references you
  touched with `git ls-files --eol -- <those paths>`: none may show `i/crlf`. The whole directory is no
  test — files committed with CRLF long ago are there already.

## 5. Run it on the second platform before you commit

A reference holding coordinates is shared by Windows and Linux, and it is not verified until the other
platform has read it. When both platforms run over one checkout, give each run its own `-o`: the runner
deletes its output directory when it starts, and a concurrent run then fails with errors that have
nothing to do with the change.

Read the header before the results: `Indigo library path` must point into this checkout. The
`Indigo version` line is stamped by `git describe` when CMake configures, so it does not prove the
library is fresh.

Where a platform genuinely lays out differently — macOS on ARM can — give that platform its own
reference rather than weakening the test:

```bash
: > tests/<group>/ref/<platform>/<name>.ket    # bootstrap: -u only updates a file that exists
python api/tests/integration/test.py -p '<group>/*' -t 1 -o <out dir> -u   # on that platform
```

Then re-run on every platform — the platform file must not change another one's result. A difference
between Windows and Linux is not such a case: it means a float argument reached `cos`, `atan2` or `hypot`
in the layout ([invariants.md](../../../.memory-bank/invariants.md), A10), and that is fixed in the code.

## 6. Prove the dialect, then the formatting

```bash
java -jar jython-standalone-2.7.2.jar -c "compile(open('<test>.py','rb').read().decode('utf-8'), '<test>.py', 'exec')"

docker run --rm -v "$PWD":/src -w /src epmlsop/indigo-tester:latest \
  bash -c 'python3 -m pip install -r api/python/requirements_dev.txt --break-system-packages && sh -eux .ci/static_analysis_check.sh'
```

The container is the one CI uses, and its `black` is 23.3.0 reading `line_length = 79` from the root
`pyproject.toml` — a locally installed `black` with different defaults will pass a file CI rejects.

## 7. Report

In the commit message or the PR description, one line per reference:

```text
tests/<group>/ref/<name>.ket   new        <what it pins>
tests/<group>/ref/<name>.ket   changed    <what changed in the behaviour, and why the new value is right>
ref/<group>/<subject>.py.out   new
verified: Windows, Linux, Jython compile, static analysis
```

## Traps

- Without a `.out` the runner compares nothing and reports the test as `[NEW]` — easy to miss in a long
  run, though it sets bit 4 of the exit code (1 is a failure, 2 an error). `compare_diff` against a
  missing data file raises instead. Check the `.out` was created before calling the test green.
- Jython and IronPython read the same `.out`. Two wrapper differences are already known: coordinates
  arrive in IronPython as `System.Single` and need `float()`, and a value printed at a precision it rounds
  exactly on (`%.2f` of 1.875) prints differently wherever the last bit does. Anything else a wrapper
  prints differently is a wrapper defect, not a reason for a `jython/` or `iron/` reference.

## Checklist

- [ ] The reference is the assertion; nothing is computed in the test
- [ ] Fixtures in `tests/<group>/molecules/`, references in `tests/<group>/ref/`, `.out` created — no `[NEW]`
- [ ] `-u` was the last option and the result line appeared
- [ ] A reason for every changed reference; no output left in the source tree; no `i/crlf` on the references you touched
- [ ] Green without `-u` on Windows and on Linux, each with its own `-o`
- [ ] No per-platform reference between Windows and Linux; any other one justified in the report
- [ ] Compiles under Jython; static analysis passes in the CI container
