## Bingo API for using with Elasticsearch 

**IN DEVELOPMENT**

This SDK is intended to:

- Read molecules from SDF, Smiles, Mol, CML files, etc
- Index them into Elasticsearch
- Have ability to search molecules efficiently with different similarity metrics (Tanimoto, Tversky, Euclid)
- Filter additionally based on text or number fields attached to the records

#### Supported Elasticsearch versions and distributions

We are supporting 7.10.x Elasticsearch and most major distributions available (AWS, Elastic, OpenDistro, etc)

*TBD test against other 7.x versions*

### Installation

#### Dependency

Add dependency to your Maven POM file like this:

```
<dependency>
    <groupId>com.epam.indigo</groupId>
    <artifactId>bingo-elastic</artifactId>
    <version>VERSION</version>
</dependency>
```

Gradle:

```
compile group: 'com.epam.indigo', name: 'bingo-elastic', version: 'VERSION'
```

it will work the same for other major dependency managers

#### Elastisearch installation

You could use any favourite Elasticsearch distribution:

- [Open Distro Elasticsearch](https://opendistro.github.io/for-elasticsearch-docs/docs/install/)
- [Elasticsearch](https://www.elastic.co/guide/en/elasticsearch/reference/current/install-elasticsearch.html)
- many many more available on premise and as cloud products & services

Something simple could be done as following:

```
docker run -p 9200:9200 --env "discovery.type=single-node" --env "opendistro_security.disabled=true" amazon/opendistro-for-elasticsearch:latest
```

### Usage 

#### Create ElasticRepository

```
ElasticRepositoryBuilder<IndigoRecord> builder = new ElasticRepositoryBuilder<>();
        repository = builder
                .withHostName("localhost")
                .withPort(9200)
                .withScheme("http")
                .build();
```

Other customisations like SSL, custom number of shards/replicas, refresh interval, and many more are supported

#### Read Indigo records from file

```
List<IndigoRecord> records = Helpers.loadFromCmlFile("/tmp/file.cml");
```

#### Index records into Elasticsearch

```
repository.indexRecords(records);
```

*CAVEAT*: Elasticsearch doesn't have strict notion of commit, so records might appear in the index later on

Read more about it here -  https://www.elastic.co/guide/en/elasticsearch/reference/master/index-modules.html#index-refresh-interval-setting

#### Retrieve similar records from Elasticsearch

```
List<IndigoRecord> similarRecords = repository.stream()
                    .filter(new SimilarityMatch<>(target))
                    .limit(20)
                    .collect(Collectors.toList());
```

In this case we requested top-20 most similar molecules compared to `target` based on Tanimoto similarity metric

#### Find exact records from Elasticsearch

```
List<IndigoRecord> exactRecords = repository.stream()
                    .filter(new ExactMatch<>(target))
                    .limit(20)
                    .collect(Collectors.toList())
                    .stream()
                    .filter(ExactMatch.exactMatchAfterChecker(target, indigo))
                    .collect(Collectors.toList());
```

In this case we requested top-20 candidate molecules with exact same fingerprint to `target`. After that we used `ExactMatch.exactMatchAfterChecker`, 
which double checked exact match based on actual molecule

#### Subsctructure match of the records from Elasticsearch

```
List<IndigoRecord> substructureMatchRecords = repository.stream()
                   .filter(new SubstructureMatch<>(target))
                   .limit(20)
                   .collect(Collectors.toList())
                   .stream()
                   .filter(SubstructureMatch.substructureMatchAfterChecker(target, indigo))
                   .collect(Collectors.toList());
```

In this case we requested top-20 candidate molecules with exact same fingerprint to `target`. After that we used `SubstructureMatch.substructureMatchAfterChecker`, 
which double checked substructure match based on actual molecule and it's graph representation

#### Custom fields for molecule records

Indexing records with custom text tag

```
List<IndigoRecord> indigoRecordList = Helpers.loadFromSdf("src/test/resources/rand_queries_small.sdf");
IndigoRecord indigoRecord = indigoRecordList.get(0);
indigoRecord.addCustomObject("tag", "test");
repository.indexRecord(indigoRecord);
```

Searching similar molecules to the target and filtering only those that have value of the `tag` equals to `test`

```
List<IndigoRecord> similarRecords = repository.stream()
                    .filter(new TanimotoSimilarityMatch<>(target))
                    .filter(new KeywordQuery<>("tag", "test"))
                    .collect(Collectors.toList());
```

you could also use similarly wildcard and range queries
