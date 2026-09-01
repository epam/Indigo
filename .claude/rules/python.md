---
paths:
  - "api/python/**/*.py"
  - "utils/indigo-service/**/*.py"
  - "bingo/bingo-elastic/python/**/*.py"
  - "bingo/tests/**/*.py"
  - "api/tests/**/*.py"
description: "Python standards for the Indigo wrapper, the REST service, Bingo tests and the Elastic client"
---

# Python rules

How to run each suite: [.memory-bank/testing.md](../../.memory-bank/testing.md). Formatter and
linter settings: [.memory-bank/conventions.md](../../.memory-bank/conventions.md).

## Baseline

- Python 3.9 or newer. Type hints on every public function; docstrings describe the **contract**
  (arguments, return, exceptions raised), not a retelling of the body.
- `black` (line length 79) and `isort` (black profile) are not negotiable; `flake8`, `mypy` and
  `pylint` gate the CI. `api/tests/integration` is excluded from flake8.

## Wrapper (`api/python/`)

- FFI is `ctypes` — not `cffi`, not Cython. Each `IndigoObject` method is a thin proxy over the
  corresponding C entry point.
- A C call returning `-1` is an error: raise, never return the sentinel to the caller.
- Release native resources explicitly. `__del__` calling `indigoFree()` is a backstop, not the plan.

## REST service (`utils/indigo-service/`)

- One Indigo instance per request, isolated via `ContextVar`; instances are never shared across
  requests or threads.
- Domain errors map to `400`, validation errors to `422`, and the message reaching the client is the
  Indigo message — do not rewrite it in the handler.

## Tests

- `pytest`, with `@pytest.mark.parametrize` instead of copied test bodies.
- Every error path a change introduces gets a test that asserts on the error, not merely on failure.
- Bingo tests run from `bingo/tests/` (invariant B4) and adapters **return** exceptions rather than
  raising them (invariant B3).

## Dependencies

- Pin exact versions in `requirements.txt`; express ranges in `setup.py` / `pyproject.toml`.
- An upgrade touches `requirements.txt`, `setup.py` and the Dockerfile in the same commit, or it is
  not an upgrade.

## Anti-patterns

- `import *`; mutable default arguments; bare `except:`; `print()` used as logging.
- A comment noting that two places must be kept in sync — make one of them derive from the other.
