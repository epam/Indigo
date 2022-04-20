## Bingo API for using with Elasticsearch

**IN DEVELOPMENT**

This SDK is intended to:

- Read molecules from SDF, Smiles, Mol, CML files, etc
- Index them into Elasticsearch
- Have ability to search molecules efficiently with different similarity metrics (Tanimoto, Tversky, Euclid)
- Filter additionally based on text or number fields attached to the records

#### Supported Elasticsearch versions and distributions

We are supporting 7.15.x Elasticsearch and most major distributions available (AWS, Elastic, OpenDistro, etc)

*TBD test against other 7.x versions*

### Installation

#### Dependency

Install dependency using pip

```
pip install bingo-elastic
```

Install async version

```
pip install bingo-elastic[async]
```

bingo-elastic async version supports all the same methods to index and search
molecules as sync. To use async version, just instantiate `AsyncElasticRepository`


#### Elastisearch installation

You could use any favourite Elasticsearch distribution:

- [Open Distro Elasticsearch](https://opendistro.github.io/for-elasticsearch-docs/docs/install/)
- [Elasticsearch](https://www.elastic.co/guide/en/elasticsearch/reference/current/install-elasticsearch.html)
- [OpenSearch](https://opensearch.org/docs/latest/opensearch/install/index/)
- many many more available on premise and as cloud products & services

Something simple could be done as following:

```
docker run -p 9200:9200 -p 9300:9300 -e "discovery.type=single-node" -e "indices.query.bool.max_clause_count=4096" docker.elastic.co/elasticsearch/elasticsearch:7.15.1
```

### Usage

#### Create ElasticRepository

Sync

```
repository = ElasticRepository(IndexName.BINGO_MOLECULE, host="127.0.0.1", port=9200)
```

Async

```
repository = AsyncElasticRepository(IndexName.BINGO_MOLECULE, host="127.0.0.1", port=9200)
...
repository.close()
```

Async version also supports async context manager to auto close connections:

```
async with AsyncElasticRepository(IndexName.BINGO_MOLECULE, host="127.0.0.1", port=9200) as rep:
    ...
```

Using with FastAPI:

```
app = FastAPI()
rep = AsyncElasticRepository(IndexName.BINGO_MOLECULE, host="127.0.0.1", port=9200)

# This gets called once the app is shutting down.
@app.on_event("shutdown")
async def app_shutdown():
    await rep.close()
```


Other customisations like SSL, custom number of shards/replicas, refresh interval, and many more are supported by `ElasticRepository` and `AsyncElasticRepository`

#### Read Indigo records from file

IndigoRecord can be created from IndigoObject.

Full usage example:
```
from bingo_elastic.model.record import IndigoRecord
from indigo import Indigo

indigo = Indigo()
compound = indigo.loadMoleculeFromFile("composition.mol")
indigo_record = IndigoRecord(indigo_object=compound)
```

`bingo_elastic` provides helpers to load sdf, cml, smiles and smi files

```
from bingo_elastic.model import helpers

sdf = helpers.iterate_sdf("compounds.sdf")
cml = helpers.iterate_cml("compounds.cml")
smi = helpers.iterate_smiles("compounds.smi")
```

Also function `helpers.iterate_file(file: Path)` is available. This function
selects correct iterate function by file extension. The `file` argument must
be `pathlib.Path` instance

```
from bingo_elastic.model import helpers
from pathlib import Path

sdf = helpers.iterate_file(Path("compounds.sdf"))
```


#### Index records into Elasticsearch

Full usage example sync:

```
from bingo_elastic.model import helpers
from bingo_elastic.elastic import, ElasticRepository IndexName
from pathlib import Path

repository = ElasticRepository(IndexName.BINGO_MOLECULE, host="127.0.0.1", port=9200)
sdf = helpers.iterate_file(Path("compounds.sdf"))
repository.index_records(sdf)
```

Full usage example async:

_Async indexing and search requires event loop created_

```
import asyncio
from bingo_elastic.model import helpers
from bingo_elastic.elastic import AsyncElasticRepository, IndexName
from pathlib import Path

async def index_compounds():
    repository = AsyncElasticRepository(IndexName.BINGO_MOLECULE, host="127.0.0.1", port=9200)
    sdf = helpers.iterate_file(Path("compounds.sdf"))
    await repository.index_records(sdf)

asyncio.run(index_compounds)

```

*CAVEAT*: Elasticsearch doesn't have strict notion of commit, so records might appear in the index later on
Read more about it here -  https://www.elastic.co/guide/en/elasticsearch/reference/master/index-modules.html#index-refresh-interval-setting

For indexing one record the the method `ElasticRepository.index_record` can be used

#### Retrieve similar records from Elasticsearch

Sync:
```
from bingo_elastic.predicates import SimilarityMatch
alg = SimilarityMatch(target, 0.9)
similar_records = repository.filter(similarity=alg, limit=20)
```

Async:
```
from bingo_elastic.predicates import SimilarityMatch
alg = SimilarityMatch(target, 0.9)
similar_records = await repository.filter(similarity=alg, limit=20)
```

In this case we requested top-20 most similar molecules compared to `target` based on Tanimoto similarity metric

Supported similarity algorithms:
- `SimilarityMatch` or `TanimotoSimilarityMatch`
- `EuclidSimilarityMatch`
- `TverskySimilarityMatch`

#### Find exact records from Elasticsearch

Sync:
```
exact_records = repository.filter(exact=target, limit=20)
```

Async:
```
exact_records = await repository.filter(exact=target, limit=20)
```

In this case we requested top-20 candidate molecules with exact same fingerprint to `target`.
`target` should be an instance of `IndigoRecord` class.


#### Subsctructure match of the records from Elasticsearch

Sync:
```
submatch_records = repository.filter(substructure=target)
```

Async:
```
submatch_records = await repository.filter(substructure=target)
```

In this case we requested top-10 candidate molecules with exact same fingerprint to `target`.

#### Custom fields for molecule records

_Async protocol exact same, just don't forget to `await`_

Indexing records with custom fields

```
indigo_record = IndigoRecord(indigo_object=compound)
indigo_record.chembl_id = "CHEMBL2063090"
indigo_record.compound_key = "GRAZOPREVIR"
indigo_record.internal_id = 10001
```

Searching similar molecules to the target and filtering only those that have value of the `chembl_id` equals to `CHEMBL2063090`

```
from bingo_elastic.queries import KeywordQuery

alg = TanimotoSimilarityMatch(target)
result = elastic_repository.filter(similarity=alg,
                                   chembl_id=KeywordQuery("CHEMBL2063090"))
```

Or you can just write:

```
result = elastic_repository.filter(similarity=alg,
                                   chembl_id=RangeQuery(1, 10000))
```


You could also use similarly wildcard and range queries

```
from bingo_elastic.queries import WildcardQuery

result = elastic_repository.filter(chembl_id=WildcardQuery("CHEMBL2063*"))

```

```
from bingo_elastic.queries import RangeQuery

result = elastic_repository.filter(internal_id=RangeQuery(1000, 100000))

```
