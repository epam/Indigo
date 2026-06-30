# Bingo-Elastic (Python)

The standalone Python library at `bingo/bingo-elastic/python/` that indexes Indigo molecules/reactions into Elasticsearch and queries them. **Distinct** from `bingo/tests/` and the `--db bingo-elastic` adapter — that adapter exercises the cartridge test parity matrix; this library is its own product with its own API surface and its own test suite.

## Module map

- `bingo_elastic/elastic.py` — `ElasticRepository` and `AsyncElasticRepository` (parallel sync/async classes, both take a `tau_search: bool` flag), `IndexName` enum (`BINGO_MOLECULE`, `BINGO_REACTION`, `BINGO_CUSTOM`), `build_index_body(tau_search)` builder for the index mapping, and `compile_query` (dispatches a query subject + kwargs to the right query class; reroutes substructure to the tautomer path when `options` contains `TAU`).
- `bingo_elastic/queries.py` — `CompilableQuery` hierarchy: `SubstructureQuery`, `TautomerSubstructureQuery` (subclass swapping in the `sub-tau` fingerprint and `tau_fingerprint` field), `ExactMatch`, similarity matches (`TanimotoSimilarityMatch`, `EuclidSimilarityMatch`, `TverskySimilarityMatch`), plus `KeywordQuery`, `RangeQuery`, `WildcardQuery` for non-chemical fields. `query_factory` maps kwarg keys (`"substructure"`, `"tautomer"`, `"exact"`, …) to a class.
- `bingo_elastic/model/record.py` — `IndigoRecord` (abstract), `IndigoRecordMolecule`, `IndigoRecordReaction`, and the `WithIndigoObject` descriptor that extracts fingerprints + `cmf` + `hash` from an `IndigoObject` at construction time. The descriptor also computes the `sub-tau` fingerprint when the record was built with `tau_search=True`.
- `bingo_elastic/model/helpers.py` — file iterators (`iterate_file`, `load_reaction`).
- `tests/` — its own pytest suite with `conftest.py` fixtures that connect to `localhost:9200`.

## Core flow (the non-obvious bit)

Substructure/exact search is **two-stage**:

1. **Candidate retrieval.** A fingerprint bit-set is computed for the query molecule. Elasticsearch returns every record whose stored fingerprint is a **superset** of the query's bits — fast, indexable, no false negatives, many false positives. The math contract: if A is a substructure of B then `bits(A) ⊆ bits(B)`.
2. **Postprocess.** For each candidate, run the real subgraph-isomorphism check (`indigo.substructureMatcher(mol, options).match(query)` or `indigo.exactMatch(target, query, options)`) and drop false positives. The `options` argument flows from `filter(..., options=...)` straight into the matcher — Indigo parses `TAU`, `R*`, `INCHI`, `HYD`, etc. itself.

The `postprocess_actions` list in `compile_query` is what carries this second-stage callback. Always add the matcher to it when a query class is structural.

When `options` contains the `TAU` token, `compile_query` also switches the **candidate** ES query from `sub_fingerprint` to `tau_fingerprint` (via `TautomerSubstructureQuery`) so tautomer-equivalent records are returned for confirmation. Exact match needs no rerouting — its candidate set comes from the `hash` field, and `TAU` is consumed only by `indigo.exactMatch` in postprocess. Querying with `TAU` against a repo created with `tau_search=False` raises `ValueError` rather than silently returning nothing.

## Fingerprint types

`WithIndigoObject.__set__` (record.py) computes `"sim"` and `"sub"` unconditionally, and `"sub-tau"` when the record carries `tau_search=True`. Fingerprints are stored as bit-position lists in `sim_fingerprint` / `sub_fingerprint` / `tau_fingerprint` and indexed as `keyword` with `similarity: boolean`. The bit-position string from Indigo is `mol.fingerprint(type_).oneBitsList()`. Indigo's other types (`"sub-res"`, `"full"`) are still not wired in.

The tautomer path is opt-in on **both sides**: `tau_search=True` on the record gates fingerprint extraction, and `tau_search=True` on the repository declares `tau_fingerprint` in the index mapping (see `build_index_body`). Old records and old indexes stay byte-for-byte unchanged when these flags are left at their defaults.

Side effect to remember: fingerprints are computed at **record construction time** in the `WithIndigoObject` descriptor, not at `index_records()` time. By the time records reach the repo, the fingerprint is already frozen on the instance.

## Tests

Run from `bingo/bingo-elastic/python/`:

```bash
pip install -e .            # or whatever the project uses
pytest tests/               # all
pytest tests/test_elastic.py -k similarity   # filter
```

Requires an Elasticsearch instance reachable on `127.0.0.1:9200`. The `clear_index` fixture in `tests/conftest.py` deletes both `bingo-molecules` and `bingo-reactions` between tests, so do not point this at a populated cluster.

For spinning up Elasticsearch see [claude-docs/testing.md](testing.md).

## Sync/Async parity

`ElasticRepository` and `AsyncElasticRepository` are independent classes — there is no shared base. Any signature or behavior change on one must be mirrored on the other (constructors, `filter`, `index_records`, `index_record`). Tests pair every sync `test_*` with an async `test_a_*`; follow that pattern.

## Java sibling

`bingo/bingo-elastic/java/` is a parallel implementation and lags Python — for example, tautomer search is now implemented on the Python side (issue #235) while `IndigoRecord.java:29` still carries `// TODO add tau fingerprint`. When changing one library check whether the other has a TODO referring to the same gap; the Python side is now the lead reference for tautomer.

## Related

- [claude-docs/architecture.md](architecture.md) — the `bingo/tests/` adapter pattern (different layer)
- [claude-docs/testing.md](testing.md) — Elasticsearch Docker recipe and the `--db bingo-elastic` cartridge adapter tests
- [claude-docs/build.md](build.md) — `BUILD_BINGO_ELASTIC` CMake flag
