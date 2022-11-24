package com.epam.indigo;

import com.epam.indigo.elastic.ElasticRepository;
import com.epam.indigo.elastic.ElasticsearchVersion;
import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.model.IndigoRecordMolecule;
import com.epam.indigo.model.IndigoRecordReaction;
import com.epam.indigo.model.NamingConstants;
import org.testcontainers.elasticsearch.ElasticsearchContainer;
import org.testcontainers.utility.DockerImageName;

import java.util.Arrays;
import java.util.Collections;

/**
 * This class provides elastic connection for any tests based on Elasticsearch
 */
public class BaseElasticTest {

    public static Indigo indigo;
    public static ElasticsearchContainer elasticsearchContainer;
    public static ElasticRepository<IndigoRecordReaction> repositoryReaction;
    public static ElasticRepository<IndigoRecordMolecule> repositoryMolecule;

    protected static void setUpRepository(Class<? extends IndigoRecord> recordClassRepository) throws Exception {
        if (elasticsearchContainer == null) {
            indigo = new Indigo();
            elasticsearchContainer = new ElasticsearchContainer(
                    DockerImageName
                            .parse(ElasticsearchVersion.DOCKER_IMAGE_NAME)
                            .withTag(ElasticsearchVersion.VERSION)
            ).withEnv("indices.query.bool.max_clause_count", "4096").withEnv("xpack.security.enabled", "false");

            elasticsearchContainer.start();
        }
        String className = recordClassRepository.getName();
        if (className.equals("com.epam.indigo.model.IndigoRecordReaction")) {
            ElasticRepository.ElasticRepositoryBuilder<IndigoRecordReaction> builder
                    = new ElasticRepository.ElasticRepositoryBuilder<>();
            repositoryReaction = builder.withIndexName(NamingConstants.BINGO_REACTIONS)
                    .withHostsNames(Collections.singletonList(elasticsearchContainer.getHost()))
                    .withPort(elasticsearchContainer.getFirstMappedPort())
                    .withScheme("http")
                    .withRefreshInterval("1s")
                    .build();
        } else if (className.equals("com.epam.indigo.model.IndigoRecordMolecule")) {
            ElasticRepository.ElasticRepositoryBuilder<IndigoRecordMolecule> builder
                    = new ElasticRepository.ElasticRepositoryBuilder<>();
            repositoryMolecule = builder.withIndexName(NamingConstants.BINGO_MOLECULES)
                    .withHostsNames(Collections.singletonList(elasticsearchContainer.getHost()))
                    .withPort(elasticsearchContainer.getFirstMappedPort())
                    .withScheme("http")
                    .withRefreshInterval("1s")
                    .build();
        } else {
            throw new Exception("Unsupported class " + className);
        }
    }
}
