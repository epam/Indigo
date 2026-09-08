# Architecture

> **Read when:** you need to know where a component lives, where a change of a given kind belongs,
> or how something reaches the language wrappers.
> **Skip when:** you already know which file you are editing.

Verified on `f0cc3c423`.

## Shape

One C++ core, one C API over it, and everything else on the far side of that boundary.

```
core/indigo-core/        graph, molecules, reactions, loaders/savers, layout, common utilities
core/render2d/           2D rendering over the core

api/c/                   the FFI boundary — four shared libraries:
    indigo/                the main API
    indigo-inchi/          InChI conversion
    indigo-renderer/       rendering
    bingo-nosql/           embedded structure search
api/cpp/                 a C++ wrapper over the C API
api/python/  api/java/  api/dotnet/  api/r/  api/wasm/     language bindings
api/http/                FastAPI REST service over the Python binding
api/tests/integration/   the cross-language conformance harness

bingo/bingo-core/  bingo-core-c/        search engine shared by the cartridges
bingo/postgres/  oracle/  sqlserver/    per-database cartridges
bingo/bingo-elastic/{java,python}       Elasticsearch clients (separate products)
```

## The rules that hold it together

**The C API is the only FFI boundary, and its ABI is stable.** Every binding delegates to it; none
reaches past it into the core, and none reimplements chemistry. A capability that is not in the C
API does not exist for Python, Java, .NET, R or WASM.

**The core does not know the wrappers exist.** Dependencies run one way:
`core → api/c → wrappers → service`. `render2d` sits beside the core and is reached through
`indigo-renderer`.

**State lives in a session.** Objects are handles allocated inside a session; a handle from one
session is meaningless in another. This is what makes the HTTP service safe to run with one Indigo
instance per request, and it is why nothing caches a handle in a global.

**Options are session state, not arguments.** Behaviour that varies — the valence model, loader
strictness, rendering parameters — is set through the option system and read at the point of use.
Adding an option means registering it in `api/c/indigo/src/indigo_options.cpp` and reading it where
it applies; adding a parameter to one function instead is how the same setting ends up configurable
in two incompatible ways.

## How a structure moves through the system

Loading and saving are symmetric families in `core/indigo-core/molecule/` and `reaction/`: one
`*_loader` and one `*_saver` per format, over a common `BaseMolecule`. Everything in between —
matching, canonicalisation, layout, rendering, fingerprints — operates on that shared model, which
is why a new piece of per-atom data has to be wired into the whole lifecycle rather than into the
one loader that produces it ([invariants.md](./invariants.md), A4 and A5).

Formats that other tools own — InChI in particular — are delegated to the vendored library rather
than reimplemented, and their quirks belong in the wrapper around them.

## Where a change belongs

| Kind of change | Where |
| --- | --- |
| An algorithm on molecules or reactions | `core/indigo-core/<area>/`, in a companion class, not in `BaseMolecule` |
| Container or ownership behaviour | `core/indigo-core/common/base_cpp/` — see [modules/base-containers.md](./modules/base-containers.md) |
| A new format, or a fix to one | the matching `*_loader.h` / `*_saver.h` pair, plus a round-trip test |
| Something callers must be able to invoke | `api/c/` first, then every wrapper — a method in one binding only is a support liability |
| Rendering | `core/render2d/`, remembering that the WASM backend is lunasvg, not cairo |
| Database-side search | `bingo/bingo-core/` if it is shared; a cartridge directory only if it is genuinely engine-specific |
| A REST capability | `api/http/indigo_service/`, over the existing Python binding |

## Bingo

The cartridges share `bingo-core` and differ in how each database exposes it: Postgres reads its
configuration from a table, Oracle from a hand-written loader ([invariants.md](./invariants.md), B1),
and the molecule and reaction paths are parallel `mango_*` / `ringo_*` families that must be changed
together (B2). Stored structures are held in Indigo's own compact CMF form, so a change to
canonicalisation or to CMF invalidates existing indices.

`bingo/bingo-elastic/` is not a cartridge: it is two standalone client libraries, in Java and
Python, that index structures into Elasticsearch.

## Related

- [testing.md](./testing.md) — every suite, including the cross-language harness and the Bingo test
  adapter pattern that backs `--db <engine>`
- [build.md](./build.md) — how these components are configured and compiled
- [modules/](./modules/README.md) — subsystem deep dives
