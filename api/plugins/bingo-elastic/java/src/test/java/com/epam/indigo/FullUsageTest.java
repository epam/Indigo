package com.epam.indigo;

import com.epam.indigo.elastic.ElasticRepository;
import com.epam.indigo.elastic.ElasticRepository.ElasticRepositoryBuilder;
import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecord;
import com.epam.indigo.predicate.*;
import org.junit.jupiter.api.*;
import org.testcontainers.elasticsearch.ElasticsearchContainer;

import java.io.IOException;
import java.util.List;
import java.util.Random;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;

import static org.junit.jupiter.api.Assertions.*;

public class FullUsageTest {

    private static final Random random = new Random();
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
                .withRefreshInterval("1s")
                .build();
    }

    @AfterAll
    public static void tearDownElastic() {
        elasticsearchContainer.stop();
    }

    @AfterEach
    public void deleteIndex() throws IOException {
        repository.deleteAllRecords();
    }

    @Test
    @DisplayName("Testing full usage, indexing, searching")
    public void fullUsage() {
        try {
            List<IndigoRecord> indigoRecordList = Helpers.loadFromSdf("src/test/resources/rand_queries_small.sdf");
            repository.indexRecords(indigoRecordList);
            TimeUnit.SECONDS.sleep(5);
            int requestSize = 20;
            IndigoRecord target = indigoRecordList.get(random.nextInt(indigoRecordList.size()));
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

    @Test
    @DisplayName("Testing exact match")
    public void exactMatch() {
        try {
            List<IndigoRecord> indigoRecordList = Helpers.loadFromSdf("src/test/resources/rand_queries_small.sdf");
            repository.indexRecords(indigoRecordList);
            TimeUnit.SECONDS.sleep(5);
            IndigoRecord target = indigoRecordList.get(random.nextInt(indigoRecordList.size()));
            List<IndigoRecord> similarRecords = repository.stream()
                    .filter(new ExactMatch<>(target))
                    .collect(Collectors.toList());
            assertEquals(1, similarRecords.size());
            assertEquals(1.0f, similarRecords.get(0).getScore());
            assertArrayEquals(target.getFingerprint().toArray(), similarRecords.get(0).getFingerprint().toArray());
        } catch (Exception exception) {
            Assertions.fail("Exception happened during test " + exception.getMessage());
        }
    }

    @Test
    @DisplayName("Testing tversky match")
    public void tversky() {
        try {
            List<IndigoRecord> indigoRecordList = Helpers.loadFromSdf("src/test/resources/rand_queries_small.sdf");
            repository.indexRecords(indigoRecordList);
            TimeUnit.SECONDS.sleep(5);
            IndigoRecord target = indigoRecordList.get(random.nextInt(indigoRecordList.size()));
            List<IndigoRecord> similarRecords = repository.stream()
                    .filter(new TverskySimilarityMatch<>(target, 0.5f, 0.5f))
                    .collect(Collectors.toList());
            assertEquals(10, similarRecords.size());
            assertEquals(1.0f, similarRecords.get(0).getScore());
            assertArrayEquals(target.getFingerprint().toArray(), similarRecords.get(0).getFingerprint().toArray());
        } catch (Exception exception) {
            Assertions.fail("Exception happened during test " + exception.getMessage());
        }
    }

    @Test
    @DisplayName("Testing euclid with threshold match")
    public void euclidWithThreshold() {
        try {
            List<IndigoRecord> indigoRecordList = Helpers.loadFromSdf("src/test/resources/rand_queries_small.sdf");
            repository.indexRecords(indigoRecordList);
            TimeUnit.SECONDS.sleep(5);
            float threshold = 0.5f;
            IndigoRecord target = indigoRecordList.get(random.nextInt(indigoRecordList.size()));
            List<IndigoRecord> similarRecords = repository.stream()
                    .filter(new EuclidSimilarityMatch<>(target, threshold))
                    .collect(Collectors.toList());

            for (IndigoRecord similarRecord : similarRecords) assertTrue(similarRecord.getScore() >= threshold);
        } catch (Exception exception) {
            Assertions.fail("Exception happened during test " + exception.getMessage());
        }
    }

    @Test
    @DisplayName("Testing tanimoto and keyword query")
    public void keywordQueryWithTanimoto() {
        try {
            List<IndigoRecord> indigoRecordList = Helpers.loadFromSdf("src/test/resources/rand_queries_small.sdf");
            IndigoRecord indigoRecord = indigoRecordList.get(0);
            indigoRecord.addCustomObject("tag", "test");
            repository.indexRecord(indigoRecord);
            TimeUnit.SECONDS.sleep(5);
            IndigoRecord target = indigoRecordList.get(0);
            List<IndigoRecord> similarRecords = repository.stream()
                    .filter(new TanimotoSimilarityMatch<>(target))
                    .filter(new KeywordQuery<>("tag", "test"))
                    .collect(Collectors.toList());

            assertEquals(1, similarRecords.size());
        } catch (Exception exception) {
            Assertions.fail("Exception happened during test " + exception.getMessage());
        }
    }
}
