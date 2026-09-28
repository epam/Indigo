---
paths:
  - "api/tests/integration/**"
description: "How a test in the cross-language integration harness is written: reference files, fixtures, the Python dialect three interpreters share"
---

# Integration harness rules

How to run the harness: [.memory-bank/testing.md](../../.memory-bank/testing.md). Python formatting
and the linters: [python.md](./python.md).

## A test is a list of files and a comparison

The harness is a conformance suite, not a place for assertions written by hand. A new test names its
inputs and hands the output to a comparison that already exists in `common/`:

| Comparing | Helper | Behaviour |
| --- | --- | --- |
| a saved format — KET, MOL, SMILES, CDXML | `common.util.compare_diff(ref_path, filename, data)` | unified diff against the reference file; prints `SUCCEED` or the diff |
| coordinates with a tolerance | `env_indigo.moleculeLayoutDiff(indigo, mol, ref, delta)` | atom by atom, `ref/<platform>/` aware |
| anything else | `print()` | stdout is compared to the recorded `.out` |

Geometry computed inside a test — centroids, angles, distances, tolerances — is the thing to avoid:
it is code that itself needs review and it hides what actually changed. Save the structure and diff
it; the reference file shows the reviewer the whole picture. `tests/formats/mol_to_ket.py` is the
shape to copy.

## Where the files live

```
tests/<group>/<subject>.py              the test - named for the subject, no "golden", "new", "test_"
tests/<group>/molecules/<name>.ket      fixtures, shared by the whole group (also reactions/)
tests/<group>/ref/<name>.ket            data references
tests/<group>/ref/<platform>/<name>.ket a platform's own reference, when output legitimately differs
ref/<group>/<subject>.py.out            the recorded stdout
```

## References are generated, never typed

```bash
python api/tests/integration/test.py -p <group>/<subject>.py -u   # writes both .out and data refs
python api/tests/integration/test.py -p <group>/<subject>.py      # then verify it is green
```

- `compare_diff` writes in text mode, so a run on Windows leaves CRLF in the file. The repository
  keeps references as LF — convert before committing.
- A reference that moves is explained in the commit message. Regenerating to turn a suite green is
  how a defect gets recorded as expected behaviour.

## KET references pin float noise unless you stop them

```python
indigo.setOption("json-saving-pretty", True)          # a readable diff
indigo.setOption("json-use-native-precision", True)
indigo.setOption("json-set-native-precision", 3)      # decimals, not the default 6
```

Every extra decimal is another chance for the same structure to be written differently by another
compiler. Three decimals is finer than any chemistry the layout claims.

## Coordinates — generate on one platform, verify on the other

The layout calls the elementary functions on double, which is what makes Windows and Linux lay out a
structure bit for bit alike (`.memory-bank/invariants.md`, A10), so one reference serves both. It is
still verified: generate a reference that contains coordinates on one platform and run the test on
the other before committing. A new `cos` on a float argument in the layout brings the divergence back,
and this run is where it shows — for a symmetric ligand as atoms that swap positions, not as noise.

Where a platform genuinely lays out differently — macOS on ARM can, because its compiler fuses
`a * b + c` into one instruction — put that platform's file in `ref/<platform>/`; `compare_diff` and
`getRefFilepath` both prefer it, and `-u` updates it once the file exists. Lowering a check until both
platforms pass is not the alternative.

A number printed to the `.out` is printed at a precision no expected value rounds on. A stretched
bond of 1.5 × 1.25 = 1.875 printed with `%.2f` sits exactly on the boundary, and the last bit of a
float then decides the digit.

## The dialect is what three interpreters share

Every test runs under CPython 3, Jython 2.7 (the Java wrapper) and IronPython 3.4 (the .NET one), so
Python 3 only syntax fails the Java job, not the CPython one:

| Instead of | Write |
| --- | --- |
| `open(path, encoding="utf-8")` | `open(path, "rb").read().decode("utf-8")` |
| `max(values, default=0.0)` | `max(list(values) or [0.0])` |
| `bytes(renderer.renderToBuffer(mol))` | `"".join(chr(b & 0xFF) for b in buffer)` — Java hands over a signed `byte[]` |
| `x, y = atom.xyz()[0], atom.xyz()[1]` in arithmetic | `float(xyz[0])` — .NET hands over `System.Single`, and IronPython keeps single precision |
| f-strings, `pathlib`, walrus | `%` formatting, `os.path`, plain assignment |

`python3 -c` proves nothing here. Compile the file with the interpreter that will run it:

```bash
java -jar jython-standalone-2.7.2.jar -c "compile(open('<test>.py','rb').read().decode('utf-8'), '<test>.py', 'exec')"
```
