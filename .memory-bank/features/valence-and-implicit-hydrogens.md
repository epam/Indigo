# Valence and implicit hydrogens

> **Read when:** a ticket is about hydrogen counts, valence errors, or a structure that behaves
> differently after a round trip through another tool.
> **Skip when:** the work does not touch atom state.

<!-- STUB: covers the model selection and the failure contract. The per-element rules themselves are
     in the BIOVIA specifications and in valence_model.cpp; they are not restated here. -->

Anchors verified on `c57c377e4`.

## Problem

The number of hydrogens on an atom is usually not written in the file — it is derived from the
element, the charge, the radical state and the bonds actually drawn. The rules for deriving it are
not universal: BIOVIA changed them between the 2009 and 2017 specifications, and the two disagree
about real structures. Indigo therefore does not have "the" hydrogen count; it has a hydrogen count
**under a stated model**.

## Interface

`valence-mode`, a string option (`api/c/indigo/src/indigo_options.cpp:383`):

| Value | Meaning |
| --- | --- |
| `biovia-2009`, or `default` | every main-group element gets a full valence calculation |
| `biovia-2017` | only the 22 listed non-metals, plus the Al⁻ exception |

Any other value raises: `invalid valence mode: '<x>' (expected 'biovia-2009', 'biovia-2017', or
'default')` (`indigo_options.cpp:52`). Reading the option back returns `biovia-2009` or
`biovia-2017` — never `default`.

`ignore-bad-valence`, a boolean option (`indigo_options.cpp:382`), suppresses the exception described
below.

The count is read through `indigoCountImplicitHydrogens`
(`api/c/indigo/src/indigo_molecule_operations.cpp:3249`), which reports one atom's count or, given a
molecule, the sum over all atoms.

## Expected behaviour

**When** the option is unset, **then** the 2009 model applies. `default` is a synonym for
`biovia-2009`, not a third behaviour.

**When** a structure is loaded under one model and its hydrogen count read under the other, **then**
the counts may legitimately differ. This is not a bug to be reconciled; it is the reason the option
exists.

**When** an atom's drawn bonds, charge and radical state admit no valid valence, **then**
`Element::Error` is raised with `bad valence on <element> having <n> drawn bonds, charge <c>, and
<r> radical electrons` (`core/indigo-core/molecule/src/valence_model.cpp:904`) — unless
`ignore-bad-valence` is set, in which case the atom keeps an undefined hydrogen count and
`checkmolecule` is expected to surface the problem instead.

**When** a structure loads successfully, **then** nothing has been asserted about its chemical
validity: loading and validation are separate layers, and the loaders accept structures the checkers
reject.

## Guarantees

- The selected model is a property of the session's options, applied at the point the count is
  computed — not baked into the file at load time.
- The error text above is part of the public contract: integration tests and users read it verbatim.

## Limitations

- The model affects computed hydrogens only. It does not change explicit hydrogens present in the
  file, and it does not rewrite the structure.
- **A client may recompute the count itself and never show Indigo's answer.** Ketcher does exactly
  this — its canvas derives implicit hydrogens from its own JavaScript model and overwrites what
  Indigo returned, so a change of valence model is visible in exported output and not on the canvas.
  When a report says "the editor shows the wrong number of hydrogens", establish which of the two
  models produced the number before looking at this code.
