# Objects are owned by `PtrArray` and `PtrReusablePool`, not by `ObjArray` or `std::vector<T>`

**Date:** 2026-09-02 · **Status:** Accepted · **Tickets:** #3703 (PR #3727), #3766 (PR #3814)

<!-- STUB: reconstructed after the fact from the code and the analyses in
     indigo-common/Task/{milestone-19-analysis,objarray-to-ptrarray,3703,3766}/. The decisions are
     recorded here because the code now depends on them; the deliberation itself is in those files. -->

## Context

The core stored polymorphic objects in `ObjArray<T>`, a hand-written container holding objects by
value in a raw byte buffer with manual placement-new and destructor calls. It gave dense storage and
stable indices, and it cost: no exception safety during growth, no way to hold a derived type, and a
`clear()` whose destructiveness differed from the other containers in the same directory.

The obvious modernisation — replace everything with `std::vector<T>` — does not work here. Two
properties of the codebase forbid it:

- **Indices are identity.** Atoms, bonds, vertices and edges are addressed by index across the whole
  library and across the FFI boundary. A container that moves its elements on growth invalidates
  every cached pointer; a container that renumbers on removal invalidates every stored index.
- **The elements are polymorphic and expensive to construct.** `Vertex` and the graph objects carry
  their own buffers; destroying and reallocating them on every removal is measurable in parsing and
  matching workloads.

## Decision

Two containers, with different jobs:

- **`PtrArray<T>`** (`common/base_cpp/ptr_array.h`) — the owning array. Elements are held as
  `std::unique_ptr<T>` behind an index-stable interface (`add`, `emplace`, `push`, `set`, `reset`,
  `remove`, `operator[]`, `qsort`). Copying is deleted; ownership is single and explicit. This
  replaced `ObjArray<T>` everywhere, and `obj_array.h` was deleted.
- **`PtrReusablePool<T>`** (`common/base_cpp/ptr_reusable_pool.h`) — the pool for objects whose
  slots are recycled. A retired slot keeps its object alive and its buffers allocated; the object is
  returned to a freshly-constructed logical state instead of being destroyed.

The contract for pooled elements is the `Reusable` interface (`common/base_cpp/reusable.h`): a pure
virtual `reuse()` performing a **non-destructive** reset — logically equivalent to destroy plus
default-construct, minus the deallocation. The pool calls it both when a slot is retired and again
when the slot is handed back out, so an implementation must be idempotent and must leave the object
usable rather than merely consistent. It is a pure virtual rather than a compile-time trait so that
the language itself guarantees every element type provides it; it is `DLLEXPORT` so dll-interface
classes such as `Vertex` can derive from it without MSVC C4275.

`Graph` and `Vertex` both derive from `Reusable`, and `Graph` holds its vertices in a
`PtrReusablePool<Vertex>`. The pool is itself `Reusable`, so pools nest.

## Alternatives considered

- **`std::vector<T>` throughout** — rejected: reallocation invalidates cached pointers and the
  element types are polymorphic. This was the original framing of the "migrate to std containers"
  milestone, and it is the part of that milestone that does not apply.
- **`std::vector<std::unique_ptr<T>>` used directly at each call site** — rejected: it is exactly
  `PtrArray`'s backing store, but without the index-stable removal semantics, without bounds checks
  carrying the method name, and repeated at every call site.
- **A compile-time trait (`has_reuse_v`) instead of an interface** — rejected: a missing
  implementation degrades silently to "no reuse" instead of failing to compile.
- **Destroying and reconstructing pooled objects on removal** — rejected: it defeats the purpose of
  the pool, which exists because reconstruction is what costs.

## Consequences

- **Index stability is now a contract, not an accident.** Anything keyed by element index depends on
  it — which is also why a side table keyed by index must be cleared on removal (invariant A5).
- **Every pooled element type must implement `reuse()` correctly.** An implementation that clears
  the wrong subset of state produces an object that looks fresh and is not, and the failure appears
  in whatever operation next takes that slot — never at the site of the bug.
- **Memory is retained, deliberately.** Pools do not shrink; a peak allocation stays allocated for
  the lifetime of the pool. That is the trade made for the allocation savings, and it means peak
  memory, not steady-state memory, is the number to watch in a regression.
- `ObjArray<T>` no longer exists; code or patches written against it must be ported, not adapted.
