# Features

> **Read when:** you need the promised behaviour of a capability, or you are documenting one.
> **Skip when:** the question is "how is it implemented" — that is `modules/`.

Observable behaviour of a capability, from the caller's side. No function names, no file paths: a
feature file must stay true across a refactor.

## Format

- **Problem** — what it is for.
- **Interface** — how it is reached: C API function, wrapper method, CLI flag, REST endpoint, option.
- **Expected behaviour** — `WHEN <input/state> THEN <result>` scenarios, including the error cases.
- **Guarantees** — what callers may rely on (round-trip fidelity, determinism, complexity).
- **Limitations** — the known edges, with the ticket number where one exists.

## Documented

- [valence-and-implicit-hydrogens.md](./valence-and-implicit-hydrogens.md) — the `valence-mode`
  option, the failure contract, and why a client may show a different number
- [haptic-bonds.md](./haptic-bonds.md) — bonds whose endpoint is a set of atoms: what round-trips
  today (KET, rendering) and what does not (V3000, the C API, layout)

## Worth writing next

The capabilities whose behaviour is most often argued about in tickets, in rough priority order:

1. `stereochemistry` — what the loaders accept, what the checker rejects, what is silently dropped
2. `substructure-search` — matching semantics for queries, aromaticity handling, timeouts
3. `format-round-trip` — which conversions are lossless, and what is dropped where they are not
4. `structure-checker` — the check catalogue and what each code actually means
