# Base containers and pools

> **Read when:** you store objects in the core, or you are changing anything in `common/base_cpp/`.
> **Skip when:** you are only *using* an existing container through its interface.

`core/indigo-core/common/base_cpp/` holds the containers everything else is built on. Anchors
verified on `f0cc3c423`.

## Responsibility

Storage with **stable integer indices**. That is the property the rest of the library depends on:
atoms, bonds, vertices and edges are addressed by index across the core, across the wrappers, and
across the C boundary. Any container here that renumbered on removal, or moved its elements on
growth, would invalidate identity throughout the codebase.

## What is here

| Header | Holds | Removal |
| --- | --- | --- |
| `array.h` | POD-ish values, contiguous | shifts — indices are **not** stable |
| `ptr_array.h` | owned objects as `unique_ptr<T>` | leaves a hole; indices stable |
| `pool.h`, `obj_pool.h` | slots with a free list | slot recycled; indices stable |
| `ptr_reusable_pool.h` | owned objects with slot reuse | slot retired, object kept alive |
| `reusable.h` | the `Reusable` interface | — |
| `cyclic_array.h`, `reusable_obj_array.h`, `string_pool.h` | specialised | — |

`Array<T>` is the exception to the index-stability rule and is used where indices are not identity —
scratch buffers, mappings, and the FFI-facing byte arrays. Do not reach for it to store objects.

## `PtrArray<T>`

Owning, index-stable, move-only (`PtrArray(const PtrArray&) = delete`, `ptr_array.h:81`). The
element is constructed by the container (`emplace`, `push`) or handed to it as a `unique_ptr`
(`add`, `set`). `remove(idx)` clears the slot without renumbering; `reset(idx)` empties one;
`clear()` empties all. Access is bounds-checked, and the check reports the calling method's name,
which is what makes an out-of-range report actionable.

Replaced `ObjArray<T>`, which no longer exists — see
[../adr/owning-containers.md](../adr/owning-containers.md).

## `PtrReusablePool<T>` and `Reusable`

For objects whose slots recycle constantly and whose construction is expensive. A retired slot keeps
its object alive with its buffers allocated; the object is returned to a freshly-constructed
*logical* state by `Reusable::reuse()`.

`reuse()` is a pure virtual on `Reusable` (`reusable.h`), and its contract is exact:

- **Non-destructive.** Equivalent to destroy plus default-construct, minus the deallocation.
- **Called twice.** Once when the slot is retired, once when it is handed back out — so it must be
  idempotent.
- **Leaves the object usable**, not merely internally consistent.

`Graph` and `Vertex` both derive from `Reusable`; `Graph` stores its vertices in a
`PtrReusablePool<Vertex>` (`graph/graph.h:275`). `PtrReusablePool` is itself `Reusable`, so a pool
can live inside a pooled object.

## Constraints and traps

- **A `reuse()` that clears the wrong subset of state is invisible at the site of the bug.** The
  object looks freshly constructed; the wrongness surfaces in whatever operation next takes that
  slot. When adding a member to a pooled type, adding it to `reuse()` is part of adding it.
- **Slots are recycled, so indices are recycled.** A side table keyed by index and not cleared on
  removal silently reattaches to the next occupant — invariant A5, and the most common defect class
  in the core.
- **Pools do not shrink.** Peak memory stays allocated for the lifetime of the pool. That is the
  deliberate trade; measure peak, not steady state, when assessing a change here.
- **`Array<T>` shifts on removal.** Mixing it with index-keyed data is a bug that behaves correctly
  until the first removal.

## Dependents

Everything: the graph layer, molecules and reactions, loaders and savers, the matchers, and the
object registry behind the C API.
