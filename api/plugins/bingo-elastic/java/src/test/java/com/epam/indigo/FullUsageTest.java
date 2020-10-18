package com.epam.indigo;

import com.epam.indigo.elastic.ElasticRepository;
import com.epam.indigo.elastic.ElasticRepository.ElasticRepositoryBuilder;
import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.predicate.TanimotoSimilarityMatch;
import org.junit.jupiter.api.*;
import org.testcontainers.elasticsearch.ElasticsearchContainer;

import java.util.List;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;

import static org.junit.jupiter.api.Assertions.assertArrayEquals;
import static org.junit.jupiter.api.Assertions.assertEquals;

public class FullUsageTest {

    protected static ElasticRepository<IndigoRecord> repository;
    private static ElasticsearchContainer elasticsearchContainer;


    @BeforeAll
    public static void setUpElastic() {
        elasticsearchContainer = new ElasticsearchContainer("docker.elastic.co/elasticsearch/elasticsearch-oss:7.9.2");
        elasticsearchContainer.start();
        ElasticRepositoryBuilder<IndigoRecord> builder = new ElasticRepositoryBuilder<>();
        repository = builder
                .withHostName(elasticsearchContainer.getHost())
                .withPort(elasticsearchContainer.getFirstMappedPort())
                .withScheme("http")
                .build();
    }

    @AfterAll
    public static void tearDownElastic() {
        elasticsearchContainer.stop();
    }

    @Test
    @DisplayName("Testing full usage, indexing, searching")
    public void fullUsasge() {
        try {
            List<IndigoRecord> indigoRecordList = Helpers.loadFromSdf("src/test/resources/rand_queries_small.sdf");
            repository.indexRecords(indigoRecordList);
            TimeUnit.SECONDS.sleep(5);
            int requestSize = 20;
            IndigoRecord target = indigoRecordList.get(0);
            List<IndigoRecord> similarRecords = repository.stream()
                    .filter(new TanimotoSimilarityMatch<>(target))
                    .limit(requestSize)
                    .collect(Collectors.toList());
            assertEquals(requestSize, similarRecords.size());
            assertEquals(1.0f, similarRecords.get(0).getScore());
            assertArrayEquals(target.getFingerprint().toArray(), similarRecords.get(0).getFingerprint().toArray());
        } catch (Exception exception) {
            Assertions.fail("Exception happened during test " + exception.getMessage());
        }
    }
}
