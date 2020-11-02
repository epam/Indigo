package com.epam.indigo.elastic;

import com.epam.indigo.Bingo;
import com.epam.indigo.Indigo;
import com.epam.indigo.elastic.ElasticRepository;
import com.epam.indigo.model.IndigoRecord;
import org.testcontainers.elasticsearch.ElasticsearchContainer;

abstract public class NoSQLElasticCompareAbstract {

    protected static ElasticRepository<IndigoRecord> repository;
    protected static ElasticsearchContainer elasticsearchContainer;
    protected static Bingo bingoDb;
    protected static final Indigo indigo = new Indigo();

    public static void setUpDataStore() {
        elasticsearchContainer = new ElasticsearchContainer("docker.elastic.co/elasticsearch/elasticsearch-oss:7.9.2");
        elasticsearchContainer.start();
        ElasticRepository.ElasticRepositoryBuilder<IndigoRecord> builder = new ElasticRepository.ElasticRepositoryBuilder<>();
        repository = builder
                .withHostName(elasticsearchContainer.getHost())
                .withPort(elasticsearchContainer.getFirstMappedPort())
                .withScheme("http")
                .withReplicas(0)
                .withRefreshInterval("1s")
                .build();
        bingoDb = Bingo.createDatabaseFile(indigo, "src/test/resources/bingo_nosql", "molecule");

    }

    public static void tearDownDataStore() {
        elasticsearchContainer.stop();
    }

    public abstract void tanimoto();

    public abstract void euclid();

    public abstract void tversky();

    public abstract void exactMatch();

    public abstract void substructureMatch();
}
