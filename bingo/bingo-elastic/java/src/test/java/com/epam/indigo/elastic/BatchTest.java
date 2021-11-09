package com.epam.indigo.elastic;

import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecordMolecule;
import com.epam.indigo.model.NamingConstants;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.Test;
import org.testcontainers.elasticsearch.ElasticsearchContainer;
import org.testcontainers.utility.DockerImageName;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import static org.junit.jupiter.api.Assertions.*;


public class BatchTest {

    protected static ElasticRepository<IndigoRecordMolecule> repository;
    protected static ElasticsearchContainer elasticsearchContainer;

    @BeforeAll
    public static void setUpDataStore() {
        elasticsearchContainer = new ElasticsearchContainer(
                DockerImageName
                        .parse(ElasticsearchVersion.DOCKER_IMAGE_NAME)
                        .withTag(ElasticsearchVersion.VERSION)
        );
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
    }

    @Test
    public void batchTest() {
        String sdfFile = "src/test/resources/rand_queries_small.sdf";
        Iterable<IndigoRecordMolecule>  indigoRecords = Helpers.iterateSdf(sdfFile);
        List<List<IndigoRecordMolecule>> acc = new ArrayList<>();
        for (List<IndigoRecordMolecule> indigoRecordList : repository.splitToBatches(indigoRecords, 20)) {
            acc.add(indigoRecordList);
        }
        assertEquals(19, acc.size());
        assertEquals(11, acc.get(18).size());
    }

    @Test
    public void saveBatchTest() {
        String sdfFile = "src/test/resources/rand_queries_small.sdf";
        try {
            repository.indexRecords(Helpers.iterateSdf(sdfFile), 20);
        } catch (IOException e) {
            Assertions.fail(e);
        }
    }


}
