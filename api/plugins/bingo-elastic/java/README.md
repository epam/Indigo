## Bingo API for using with Elasticsearch 

**IN DEVELOPMENT**

This SDK is intended to:

- Read molecules from SDF, Smiles, Mol, CML files, etc
- Index them into Elasticsearch
- Have ability to search molecules efficiently with different similarity metrics (Tanimoto, Tversky, Euclid)
- Filter additionally based on text or number fields attached to the records

#### Supported Elasticsearch versions and distributions

We are supporting 7.9.x Elasticsearch and most major distributions available (AWS, Elastic, OpenDistro, etc)

*TBD test against other 7.x versions*

#### Installation

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
                    .filter(new TanimotoSimilarityMatch<>(target))
                    .limit(20)
                    .collect(Collectors.toList());
```

In this case we requested top-20 most similar molecules compared to `target` based on Tanimoto similarity metric
