# Salt detection is one implementation in the core, behind the C API

**Date:** 2026-10-08 · **Status:** Accepted · **Ticket:** #3927

## Context

`checkSalt` and `stripSalt` existed in the Python and the Java wrapper only, each with its own copy
of the sixteen SMARTS patterns and of the loop over components. The .NET wrapper had neither. Both
fixes of #3927 — a component held together by haptic bonds is not a salt, and atoms are removed by
their own indices — had to be made twice, and the two sets of wrapper tests had drifted apart: thirty
cases in Python, eighteen in Java.

`architecture.md` puts anything callers invoke in `api/c/` first, and an algorithm on molecules in a
companion class of the core.

## Decision

`core/indigo-core/molecule/molecule_salt_stripper.h#MoleculeSaltStripper` holds the patterns and the
algorithm. `indigoCheckSalt` and `indigoStripSalt` are its two entry points, and the Python, Java and
.NET wrappers call them; `stripSalt` keeps its `inplace` argument in the wrappers, which strip a
clone when it is false.

The class matches a pattern the way the wrappers did through `indigoSubstructureMatcher` and
`indigoMatch`: the component is cut out with `makeSubmolecule`, aromatized with the options of the
session when it is not aromatic already, and matched with neighbourhood counters on both sides. On
272,892 structures of the test data the old wrapper code and the new call give the same answer and
the same error text, line for line.

## Alternatives considered

- **Port the Java code to C#.** Small and local, but a third copy of the patterns, kept in step only
  by tests.
- **Implement the two functions in `api/c` on top of `IndigoMoleculeSubstructureMatcher`.** The same
  code path as before by construction, but an algorithm on molecules outside the core, out of reach
  of Bingo and of `StructureChecker`, whose `check_salt` is still a stub.
- **A `standardize` option.** Fits stripping, not the question `checkSalt` answers, and both methods
  are public names of two wrappers already.
- **Parse the patterns once per thread**, as `crippen.cpp` keeps its queries. Parsing is about half
  the time of a call on a small molecule; a call is already 2.5 to 7 times faster than the wrapper
  code was, so the cache waits for a caller that needs it.

## Consequences

- A change to the patterns or to the rule is made once, in the core.
- The wrapper tests keep two tests each for the binding; which structures are salts is recorded in
  `api/tests/integration/tests/basic/salts.py`, which runs through all three wrappers.
- `SALTS` of `indigo.indigo.salts` in the Python package and the package-private `Salts` of Java are
  gone. The Python module keeps `IONS`, a list nothing in the repository reads.
- A reaction, an atom or another object that is not a molecule is refused with `<object> is not a
  molecule`; the wrappers used to fail earlier, in `iterateComponents()`, with `<object> is not a
  base molecule`. An empty query molecule is refused as any query molecule is; the wrappers answered
  for it without matching anything.
- The C++ wrapper, the R package, the WASM module and the HTTP service have no salt calls, as before.
