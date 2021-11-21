package com.epam.indigo.elastic;

import com.epam.indigo.Bingo;
import com.epam.indigo.Indigo;
import com.epam.indigo.model.IndigoRecordMolecule;
import com.epam.indigo.model.NamingConstants;
import org.testcontainers.elasticsearch.ElasticsearchContainer;
import org.testcontainers.utility.DockerImageName;

/**
 * TODO: Add generic support or add IndigoRecordReaction support
 */
abstract public class NoSQLElasticCompareAbstract {

    protected static ElasticRepository<IndigoRecordMolecule> repository;
    protected static ElasticsearchContainer elasticsearchContainer;
    protected static Bingo bingoDb;
    protected static final Indigo indigo = new Indigo();

    public static void setUpDataStore() {
        elasticsearchContainer = new ElasticsearchContainer(
                DockerImageName
                        .parse(ElasticsearchVersion.DOCKER_IMAGE_NAME)
                        .withTag(ElasticsearchVersion.VERSION))
                .withEnv("xpack.security.enabled", "false");
        elasticsearchContainer.start();
        ElasticRepository.ElasticRepositoryBuilder<IndigoRecordMolecule> builder = new ElasticRepository.ElasticRepositoryBuilder<>();
        repository = builder
                .withIndexName(NamingConstants.BINGO_MOLECULES)
                .withHostName(elasticsearchContainer.getHost())
                .withPort(elasticsearchContainer.getFirstMappedPort())
                .withScheme("http")
                .withReplicas(0)
                .withRefreshInterval("1s")
                .build();
        bingoDb = Bingo.createDatabaseFile(indigo, "src/test/resources/bingo_nosql", "molecule");
    }

    public static void tearDownDataStore() {
        bingoDb.close();
        elasticsearchContainer.stop();
    }

    public abstract void tanimoto();

    public abstract void euclid();

    public abstract void tversky();

    public abstract void exactMatch();

    public abstract void substructureMatch();
}
