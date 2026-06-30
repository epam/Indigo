# Testing

How to run each of Indigo's test suites. Oracle has its own document because of the additional Docker harness and gotchas — see [claude-docs/oracle.md](oracle.md).

## Indigo integration tests

Requires `indigo-python` to be built and installed first:

```bash
pip install dist/epam.indigo-*.whl

# All tests
python api/tests/integration/test.py -t 1

# Filter by name pattern
python api/tests/integration/test.py -t 1 -p test_name
```

## Bingo tests

Configure DB connections in `bingo/tests/db_config.ini` before running.

```bash
pip install -r bingo/tests/requirements.txt
cd bingo/tests

pytest .                    # default (bingo-nosql)
pytest --db postgres        # PostgreSQL
pytest --db oracle          # Oracle (see claude-docs/oracle.md)
pytest --db bingo-elastic   # Elasticsearch

# Run a single test module
pytest test_exact/test_exact.py --db postgres
```

### Spinning up bingo-postgres via Docker

```bash
docker build --tag epmlsop/bingo-postgres:latest \
  -f bingo/postgres/Dockerfile --build-arg BINGO_PG_VERSION=14 .
docker run -d -p 5432:5432 -e "POSTGRES_PASSWORD=password" epmlsop/bingo-postgres:latest
```

### Spinning up Elasticsearch for bingo-elastic

```bash
docker run -p 9200:9200 \
  --env "discovery.type=single-node" \
  --env "indices.query.bool.max_clause_count=4096" \
  docker.elastic.co/elasticsearch/elasticsearch:7.17.11
```

## Backend API (indigo-service) tests

```bash
export INDIGO_SERVICE_URL=http://localhost:5000/v2
cd utils/indigo-service/backend/service && cp v2/common/config.py .

# Terminal 1
waitress-serve --listen="127.0.0.1:5000 [::1]:5000" app:app

# Terminal 2
python utils/indigo-service/backend/service/tests/api/indigo_test.py
```

## Related

- [claude-docs/oracle.md](oracle.md) — Oracle-specific Docker harness, host venv setup, and gotchas
- [claude-docs/architecture.md](architecture.md) — the Bingo test adapter pattern that backs `--db <engine>`
- [claude-docs/build.md](build.md) — building the artifacts the tests run against
